#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/bootparam.h"
#include "hal/include/fb.h"
#include "hal/include/periph.h"


/*
 * Print
 */
static int has_serial = 0;
static int has_fb = 0;

void draw_char(char ch)
{
    if (has_fb) {
        fb_draw_char(ch);
//         fb_draw_char_ppc(ch);
    }
    
    if (has_serial) {
        escc_draw_char(ch);
    }
}

void init_print()
{
    struct boot_parameters *bp = get_bootparam();
    
    switch (bp->video_mode) {
    case VIDEO_FRAMEBUFFER:
        has_fb = 1;
        init_fb_draw_char((void *)bp->fb_addr, bp->fb_res_x, bp->fb_res_y, bp->fb_bits_per_pixel >> 3, bp->fb_bytes_per_line);
//         init_fb();
        break;
    case VIDEO_SERIAL:
        has_serial = 1;
        init_escc();
        break;
    default:
        has_serial = 1;
        init_escc();
        break;
    }
}


/*
 * PIC
 */
static int is_openpic = 0;
static int is_heathrow_pic = 0;

static void init_pic()
{
//     is_openpic = 1;
    //is_heathrow_pic = 1;
    
    if (is_openpic) {
        init_openpic();
    } else if (is_heathrow_pic) {
        init_heathrow_pic();
    }
}

static void start_pic()
{
    if (is_openpic) {
        start_openpic();
    } else if (is_heathrow_pic) {
        start_heathrow_pic();
    }
}


/*
 * Top level
 */
void init_periph()
{
    struct boot_parameters *bp = get_bootparam();
    if (bp->has_openpic) {
        is_openpic = 1;
    } else {
        is_heathrow_pic = 1;
    }
    
    // OFW must be initialized first
    init_ofw();
    init_pic();
}

void start_periph()
{
    struct boot_parameters *bp = get_bootparam();
    
    if (bp->video_mode != VIDEO_FRAMEBUFFER) {
        start_escc();
    }
    start_pic();
}

int pic_get_vector()
{
    if (is_openpic) {
        return openpic_get_vector();
    } else if (is_heathrow_pic) {
        return heathrow_pic_get_vector();
    }
    
    return -1;
}

void pic_eoi(int wired)
{
    if (is_openpic) {
        openpic_eoi(wired);
    } else if (is_heathrow_pic) {
        heathrow_pic_eoi(wired);
    }
}

int pic_register_wired(int wired, int_handler handler)
{
    if (is_openpic) {
        openpic_register_wired(wired, handler);
    } else if (is_heathrow_pic) {
        heathrow_pic_register_wired(wired, handler);
    }
    
    return 0;
}
