#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/periph.h"


static int is_graphic = 0;


void draw_char(char ch)
{
    if (is_graphic) {
        fb_draw_char_ppc(ch);
    } else {
        escc_draw_char(ch);
    }
}

void init_print()
{
    struct boot_parameters *bp = get_bootparam();
    
    switch (bp->video_mode) {
    case VIDEO_FRAMEBUFFER:
        is_graphic = 1;
        init_fb();
        break;
    case VIDEO_SERIAL:
        init_escc();
        break;
    default:
        init_escc();
        break;
    }
}
