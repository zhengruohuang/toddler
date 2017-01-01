#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/periph.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"
#include "hal/include/task.h"
#include "hal/include/int.h"
#include "hal/include/cpu.h"


static void hal_entry(struct boot_parameters *boot_param)
{
    // First we init periph and screen then tell the user we are in HAL
    init_periph();
    init_video();
    kprintf("We are in HAL!\n");
    
    // Init bootparam
    init_bootparam(boot_param);
    
    // Init memory system
    init_kalloc();
    init_tlb();
    
    // Init CPU
    init_cpuid();
    init_topo();
    init_mp();
    
    // Init task
    init_context();
    
    // Init interrupt
    init_int_vector();
    init_int();
    init_syscall();
    
    // Done
    kprintf("Initialization is done! Will start working!\n");
}


/*
 * This is the entry point of HAL
 */
void asmlinkage no_opt _start(struct boot_parameters *boot_param)
{
    switch (boot_param->hal_start_flag) {
    // Start HAL
    case 0:
        hal_entry(boot_param);
        break;
        
//     // Start AP
//     case 1:
//         ap_entry();
//         break;
//         
//     // Return from BIOS invoker
//     case 2:
//         bios_return();
//         break;
        
    // Undefined
    default:
        kprintf("ERROR: Undefined HAL start flag: %u\n", boot_param->hal_start_flag);
        halt();
        break;
    }
    
    // Should never reach here
    //panic("Should never reach here!");
    halt();
}
