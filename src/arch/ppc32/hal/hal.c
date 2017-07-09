#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/periph.h"
#include "hal/include/print.h"


void no_opt hal_entry(struct boot_parameters *boot_param)
{
    init_bootparam(boot_param);
    init_print();
    kprintf("We are in HAL!\n");
    
    halt();
    while (1);
}
