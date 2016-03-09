#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"
#include "hal/include/acpi.h"
#include "hal/include/int.h"
#include "hal/include/cpu.h"
#include "hal/include/mps.h"
#include "hal/include/apic.h"


static void hal_entry()
{
    // First we init the screen then tell the user we are in HAL
    init_video();
    kprintf("We are in HAL!\n");
    
    // Init CPUID
    init_cpuid();
    
    // Init mempool
    init_kalloc();
    
    // Init ACPI
    init_acpi();
    
    // Init MPS
    init_mps();
    
    // Init interrupt
    init_int_handlers();
    init_idt();
    init_int_vector();
    
    // Init topo
    init_topo();
    
    // Init APIC
    init_apic();
    
    // Init MP
    init_mp();
    
    // Init TSS
    
    // Init GDT
    
    // Init IDT
    
    // Init kernel
    
    // Bringup BSP
    
    // Bringup APs
    
    // Start to work

    
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
