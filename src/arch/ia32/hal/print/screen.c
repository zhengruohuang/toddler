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
        }
        
        else {
        }
        
        row = char_y - 1;
    }
    
    // In text mode we need to set the cursor position
    if (!pixel) {
        u32 cursor_pos = row * char_y + col;
        
        // Cursor low port to vga INDEX register
        io_out8(0x3d4, 0x0f);
        io_out8(0x3d5, (u8)(cursor_pos & 0xff));
        
        // Cursor high port to vga INDEX register
        io_out8(0x3d4, 0x0e);
        io_out8(0x3d5, (u8)((cursor_pos >> 8) & 0xff));
    }
}

static void regular(char ch)
{
    if (pixel) {
        // draw a char
    }
    
    else {
        u32 pos = (row * char_y + col) * 2;
        
        __asm__ __volatile__
        (
            "movb   $0x7, %%ah;"
            "movw   %%ax, %%ds:(%%edi)"
            :
            : "D" (fb + pos), "a" (ch)
        );
    }
    
    col++;
    if (col >= char_y) {
        row++;
    }
}

static void backspace()
{
    if (col) {
        col--;
    } else {
        row--;
        col = char_y - 1;
    }
    
    regular(' ');
    
    if (col) {
        col--;
    } else {
        row--;
        col = char_y - 1;
    }
}

static void new_line()
{
    int i;
    for (i = 0; i < char_y - col; i++) {
        regular(' ');
    }
}

static void tab()
{
    int i;
    int new_col = col;
    new_col /= TAB_WIDTH;
    new_col++;
    new_col += TAB_WIDTH;
    
    if (new_col >= char_y) {
        new_col = char_y;
    }
    
    for (i = 0; i < new_col - col; i++) {
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

void init_screen()
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
