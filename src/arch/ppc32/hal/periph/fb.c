#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "common/include/vgafont.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"


static volatile u8 *fb;
static int width, height, depth, bpp, bpl;
static int usable_width, usable_height;
static int cursor_row = 0, cursor_col = 0;
static int chars_per_row, chars_per_col;


static void fb_draw_char_at(char ch, u32 line, u32 col)
{
    u32 offset = FONT_HEIGHT * line * bpl + FONT_WIDTH * col * bpp;
    u32 offset_x;
    
    u8 *font = vga_font[ch < 0x20 ? 0 : ch - 0x20];
    u8 cur_map;
    
    int i, j;
    for (i = 0; i < FONT_HEIGHT; i++) {
        cur_map = font[i];
        offset_x = offset;
        for (j = 0; j < FONT_WIDTH; j++) {
            if (cur_map & (0x1 << (FONT_WIDTH - 1 - j))) {
                fb[offset_x + 0] = 0xc0;
                fb[offset_x + 1] = 0xc0;
                fb[offset_x + 2] = 0xc0;
                fb[offset_x + 3] = 0xc0;
            } else {
                fb[offset_x + 0] = 0;
                fb[offset_x + 1] = 0;
                fb[offset_x + 2] = 0;
                fb[offset_x + 3] = 0;
            }
            
            offset_x += bpp;
        }
        
        offset += bpl;
    }
}

static void fb_update_cursor()
{
    if (chars_per_row && cursor_col >= chars_per_row) {
        cursor_row++;
        cursor_col = 0;
    }
    
    if (chars_per_col && cursor_row >= chars_per_col) {
        u32 move_y, offset_src, offset_dest;
        u32 x, y, d, of_src, of_dest;
        
        // Move current screen up
        move_y = (cursor_row - chars_per_col + 1) * FONT_HEIGHT;
        offset_src = move_y * bpl;
        offset_dest = 0;
        
        for (y = move_y; y < usable_height; y++) {
            of_src = offset_src;
            of_dest = offset_dest;
            
            for (x = 0; x < width; x++) {
                for (d = 0; d < bpp; d++) {
                    fb[of_dest + d] = fb[of_src + d];
                }
                
                of_src += bpp;
                of_dest += bpp;
            }
            
            offset_src += bpl;
            offset_dest += bpl;
        }
        
        // Clear the rest of the screen
        move_y = (chars_per_col - 1) * FONT_HEIGHT;
        offset_dest = move_y * bpl;
        
        for (y = move_y; y < height; y++) {
            of_dest = offset_dest;
            
            for (x = 0; x < width; x++) {
                for (d = 0; d < bpp; d++) {
                    fb[of_dest + d] = 0;
                }
                
                of_dest += bpp;
            }
            
            offset_dest += bpl;
        }
        
        // Done
        cursor_row = chars_per_col - 1;
    }
}

static void black_screen()
{
    int x, y, i;
    int offset = 0;
    int offset_x = 0;
    
    for (y = 0; y < height; y++) {
        offset_x = offset;
        
        for (x = 0; x < width; x++) {
            for (i = 0; i < bpp; i++) {
                fb[offset_x + i] = 0;
            }
            
            offset_x += bpp;
        }
        
        offset += bpl;
    }
}


void fb_draw_char_ppc(char ch)
{
    if (!fb) {
        return;
    }
    
    switch (ch) {
    case '\r':
    case '\n':
        cursor_row++;
        cursor_col = 0;
        break;
    case '\t':
        cursor_col += TAB_WIDTH;
        break;
    default:
        fb_draw_char_at(ch, cursor_row, cursor_col);
        cursor_col++;
        break;
    }
    
    fb_update_cursor();
}

void init_fb()
{
    struct boot_parameters *bp = get_bootparam();
    if (bp->video_mode != VIDEO_FRAMEBUFFER) {
        return;
    }
    
    fb = (u8 *)bp->fb_addr;
    width = bp->fb_res_x;
    height = bp->fb_res_y;
    depth = bp->fb_bits_per_pixel;
    bpp = depth >> 3;
    bpl = bp->fb_bytes_per_line;
    
    cursor_row = cursor_col = 0;
    chars_per_row = width / FONT_WIDTH;
    chars_per_col = height / FONT_HEIGHT;
    usable_width = chars_per_row * FONT_WIDTH;
    usable_height = chars_per_col * FONT_HEIGHT;
    
    black_screen();
}
