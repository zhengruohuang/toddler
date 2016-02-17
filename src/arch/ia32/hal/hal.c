#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/bootparam.h"


static void no_opt stop()
{
    do {
        __asm__ __volatile__
        (
            "hlt;"
            :
            :
        );
    } while (1);
}

static void hal_entry()
{
    
}

static void ap_entry()
{
}

static void bios_return()
{
}

void asmlinkage _start()
{
    struct boot_parameters *boot_param = (struct boot_parameters *)BOOT_PARAM_PADDR;
    
    switch (boot_param->hal_start_flag) {
    // Start HAL
    case 0:
        // Switch stack to HAL's
        // thus this function is unable to return 
        __asm__ __volatile__
        (
            "xchgw  %%bx, %%bx;"
            "movl   %%eax, %%esp;"
            :
            : "a" (0xFFC02000)
        );
        
        hal_entry();
        break;
        
    // Start AP
    case 1:
        break;
        
    // Return from BIOS invoker
    case 2:
        break;
        
    // Undefined
    default:
        stop();
        break;
    }
    
    // Should never reach here
    stop();
}
