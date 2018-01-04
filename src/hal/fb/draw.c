#include "common/include/data.h"
#include "common/include/vgafont.h"
#include "hal/include/string.h"


// Framebuffer
static volatile u8 *fb = NULL;
static int width = 640, height = 480;
static int depth = 32, pitch = 0;

// Font info
static int chars_per_row = 0, chars_per_col = 0;

// Cursor
static int cursor_row = 0, cursor_col = 0;


static void clear_screen()
{
    int y;
    int offset = 0;
    
    for (y = 0; y < height; y++) {
        memset((void *)(fb + offset), 0, pitch);
        offset += pitch;
    }
}

static void draw_char(char ch, int line, int col)
{
    int offset = FONT_HEIGHT * line * pitch + FONT_WIDTH * col * depth;
    int offset_x;
    
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
            
            offset_x += depth;
        }
        
        offset += pitch;
    }
}

static void update_cursor()
{
    if (chars_per_row && cursor_col >= chars_per_row) {
        cursor_row++;
        cursor_col = 0;
    }
    
    if (chars_per_col && cursor_row >= chars_per_col) {
        int move_y = (cursor_row - chars_per_col + 1) * FONT_HEIGHT;
        int x, y, d, of_src, of_dest;
        
        int offset_src = move_y * pitch;
        int offset_dest = 0;
        
        for (y = move_y; y < height; y++) {
            of_src = offset_src;
            of_dest = offset_dest;
            
            for (x = 0; x < width; x++) {
                for (d = 0; d < depth; d++) {
                    fb[of_dest + d] = fb[of_src + d];
                }
                
                of_src += depth;
                of_dest += depth;
            }
            
            offset_src += pitch;
            offset_dest += pitch;
        }
        
        cursor_row = chars_per_col - 1;
    }
}

void fb_draw_char(char ch)
{
    switch (ch) {
    case '\n':
    case '\r':
        cursor_row++;
        cursor_col = 0;
        break;
    case '\t':
        cursor_col /= 8;
        cursor_col = (cursor_col + 1) * 8;
        break;
    default:
        draw_char(ch, cursor_row, cursor_col);
        cursor_col++;
        break;
    }
    
    update_cursor();
}

void init_fb_draw_char(void *f, int w, int h, int d, int p)
{
    fb = f;
    width = w;
    height = h;
    depth = d;
    pitch = p;
    
    clear_screen();
}
