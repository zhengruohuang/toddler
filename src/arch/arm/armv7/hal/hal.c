#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/bootparam.h"
#include "hal/include/print.h"


static void stop(ulong val)
{
    if (!val) {
        val = 0xbeef;
    }
    
    while (1) {
        __asm__ __volatile__
        (
            "mov r0, %[reg];"
            :
            : [reg] "r" (val)
        );
    }
}

extern void init_periph();

static void start_hal(struct boot_parameters *boot_param)
{
    // Make bootparam globally accessible
    init_bootparam(boot_param);
    
    // First we init periph and screen then tell the user we are in HAL
    init_periph();
    kprintf("We are in HAL!\n");
    
    // Stop here
    while (1) {
        stop(0xbbbb);
    }
}

static void start_ap()
{
    while (1) {
        stop(0);
    }
}

void entry_func hal_entry(struct boot_parameters *boot_param)
{
    switch (boot_param->hal_start_flag) {
    // Start HAL
    case 0:
        start_hal(boot_param);
        break;
        
    // Start AP
    case 1:
        start_ap();
        break;
        
    // Undefined
    default:
        kprintf("ERROR: Undefined HAL start flag: %u\n", boot_param->hal_start_flag);
//         halt();
        break;
    }
    
    while (1) {
        stop(0);
    }
}
