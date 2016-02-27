#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/lib.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/int.h"


static void hal_entry()
{
    // First we init the screen then tell the user we are in HAL
    init_video();
    kprintf("We are in HAL!\n");
    
    // Init mempool
    init_kalloc();
    
    // Init interrupt
    init_int_handlers();
    init_idt();
    
    // Init TSS

    
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
//     kprintf("We are in HAL!\n");
    
//     do {
//         kprintf("We are in HAL!\n");
//     } while(1);
}

// static void ap_entry()
// {
// }
// 
// static void bios_return()
// {
// }

/*
 * This is the entry point of HAL
 */
void asmlinkage _start()
{
    struct boot_parameters *boot_param = get_bootparam();
    
    switch (boot_param->hal_start_flag) {
    // Start HAL
    case 0:
        // Switch stack to HAL's
        // thus this function is unable to return 
//         __asm__ __volatile__
//         (
//             "xchgw  %%bx, %%bx;"
//             "movl   %%eax, %%esp;"
//             :
//             : "a" (0xFFC02000)
//         );
        
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
        kprintf("ERROR: Undefined HAL start flag: %u\n", boot_param->hal_start_flag);
        halt();
        break;
    }
    
    // Should never reach here
    halt();
}
