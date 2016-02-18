#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/print.h"


static int pixel = 0;   // video mode, 1 = pixel, 0 = text
static int bpl = 0; // bytes per line
static int bpp = 0; // bits per pixel

// Resolution or width & height
static int width = 0, height = 0;

// Number of chars in x or y
static int char_x = 0, char_y = 0;

// Cursor
static int row = 0, col = 0;

// Video framebuffer or text buffer
static ulong fb = 0;

// VGA font
#include "hal/include/font.h"


static void update_cursor()
{
    // Too many rows, we need to drop line 0 and move line 1-N backward
    if (row >= char_y) {
        if (pixel) {
            u32 *src_start = (u32 *)(fb + bpl * vga_font_height);
            u32 *dest_start = (u32 *)fb;
            u32 *last_line = (u32 *)(fb + bpl * vga_font_height * (char_y - 1));
            u32 len = bpl * vga_font_height * (char_y - 1);
            
            int i;
            for (i = 0; i < len / 4; i++) {
                *(dest_start + i) = *(src_start + i);
            }
            
            for (i = 0; i < bpl / 4 * vga_font_height; i++) {
                *(last_line + i) = 0x0;
            }
        }
        
        else {
            u16 *src_start = (u16 *)(fb + 2 * char_x);
            u16 *dest_start = (u16 *)fb;
            u16 *last_line = (u16 *)(fb + 2 * char_x * (char_y - 1));
            u32 len = 2 * char_x * (char_y - 1);
            
            u32 i;
            for (i = 0; i < len / 2; i++) {
                *(dest_start + i) = *(src_start + i);
            }
            
            for (i = 0; i < 2 * char_x / 4; i++) {
                *(last_line + i) = 0;
            }
        }
        
        row = char_y - 1;
    }
    
    // In text mode we need to set the cursor position
    if (!pixel) {
        u32 cursor_pos = row * char_x + col;
        
        // Cursor low port to vga INDEX register
        io_out8(0x3d4, 0x0f);
        io_out8(0x3d5, cursor_pos & 0xff);
        
        // Cursor high port to vga INDEX register
        io_out8(0x3d4, 0x0e);
        io_out8(0x3d5, (cursor_pos >> 8) & 0xff);
    }
}

static void regular(char ch)
{
    if (pixel) {
        u32 offset = (vga_font_height * row) * bpl + (vga_font_width * col) * (bpp / 8);
        u8 *font = vga_font[ch < 0x20 ? 0 : ch - 0x20];
        
        unsigned char cur_map;
        u32 x;
        
        u8 *buf = (u8 *)fb;
        
        int i, j;
        for (i = 0; i < 16; i++) {
            cur_map = font[i];
            x = offset;
            for (j = 0; j < 8; j++) {
                if (cur_map & (0x1 << (7 - j))) {
                    buf[x + 0] = 0xc0;
                    buf[x + 1] = 0xc0;
                    buf[x + 2] = 0xc0;
                } else {
                    buf[x + 0] = 0;
                    buf[x + 1] = 0;
                    buf[x + 2] = 0;
                }
                
                x += bpp / 8;
            }
            
            offset += bpl;
        }
    }
    
    else {
        u32 pos = (row * char_x + col) * 2;
        
        
        __asm__ __volatile__
        (
            "movb   $0x7, %%ah;"
            "movw   %%ax, %%ds:(%%edi)"
            :
            : "D" (fb + pos), "a" (ch)
        );
    }
    
    col++;
    if (col >= char_x) {
        row++;
        col = 0;
    }
}

static void backspace()
{
    if (col) {
        col--;
    } else {
        row--;
        col = char_x - 1;
    }
    
    regular(' ');
    
    if (col) {
        col--;
    } else {
        row--;
        col = char_x - 1;
    }
}

static void new_line()
{
    int i;
    int count = char_x - col;
    for (i = 0; i < count; i++) {
        regular(' ');
    }
}

static void tab()
{
    int i;
    int old_col = col;
    int new_col = col;
    new_col /= TAB_WIDTH;
    new_col++;
    new_col *= TAB_WIDTH;
    
    if (new_col >= char_x) {
        new_col = char_x;
    }
    
    for (i = 0; i < new_col - old_col; i++) {
        regular(' ');
    }
}

void draw_char(char ch)
{
    switch (ch) {
    case '\r':
    case '\n':
        new_line();
        break;
    case '\t':
        tab();
        break;
    case '\b':
        backspace();
        break;
    default:
        regular(ch);
        break;
    }
    
    update_cursor();
}

void init_video()
{
    struct boot_parameters *bp = get_bootparam();
    
    pixel = bp->video_mode;
    bpl = bp->bytes_per_line;
    bpp = bp->bits_per_pixel;
    fb = bp->framebuffer_addr;
    width = bp->res_x;
    height = bp->res_y;
    
    row = bp->cursor_row;
    col = bp->cursor_col;
    
    if (pixel) {
        char_x = width / vga_font_width;
        char_y = height / vga_font_height;
    } else {
        char_x = width;
        char_y = height;
    }
}
