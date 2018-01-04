#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/bootparam.h"
#include "hal/include/fb.h"


/*
 * Print
 */
static int fb_enabled = 0;

void draw_char(char c)
{
    if (fb_enabled) {
        fb_draw_char(c);
    }
}

static void init_print()
{
    struct boot_parameters *bp = get_bootparam();
    
    if (bp->video_mode == VIDEO_FRAMEBUFFER) {
        init_fb_draw_char((void *)bp->framebuffer_addr,
                          bp->res_x, bp->res_y, bp->bytes_per_pixel, bp->bytes_per_line);
        fb_enabled = 1;
    }
}

void init_periph()
{
    init_print();
}
