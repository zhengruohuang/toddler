#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/periph.h"


void no_opt hal_entry(struct boot_parameters *boot_param)
{
    init_bootparam(boot_param);
//     init_fb();
//     
//     int i;
//     for (i = 0; i < 1000; i++) {
//         fb_draw_char('a');
//     }
    
    init_escc();
    
    int i;
    for (i = 0; i < 1000; i++) {
        escc_draw_char('a');
    }
    
    halt();
    while (1);
}
