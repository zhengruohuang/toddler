#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/bootparam.h"
#include "hal/include/periph.h"
#include "hal/include/print.h"
#include "hal/include/kalloc.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/vector.h"
#include "hal/include/int.h"
#include "hal/include/pic.h"
#include "hal/include/kernel.h"


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


/*
 * Primary processor
 */
static void start_hal(struct boot_parameters *boot_param)
{
    // Make bootparam globally accessible
    init_bootparam(boot_param);
    
    // First we init periph and screen then tell the user we are in HAL
    init_periph();
    kprintf("We are in HAL!\n");
    
    // Init memory management
    init_kalloc();
    init_map();
    
    // Init CPU
    init_cpuid();
    init_topo();
    init_mp();
    
    // Init interrupt
    init_int_vector();
    init_int();
    init_syscall();
    init_context();
    
    // Init timer
    init_generic_timer();
    
    // Init kernel
    init_kmem_zone();
    init_kernel();
    
    // Init MP
    bringup_mp();
//     while (1);
    
    // Start working
    kprintf("Initialization done, will start working!\n");
    
    release_mp_lock();
    start_working();
    
    // Stop here
    while (1) {
        stop(0xbbbb);
    }
}


/*
 * Secondary processors
 */
static void start_ap()
{
    kprintf("HAL starting AP\n");
    
    // Init interrupt
    init_int_mp();
    init_context_mp();
    
    // Init timer
    init_generic_timer_mp();
    
    // Init is done, wait for MP lock to release
    ap_init_done();
    
    // Start working
    start_working_mp();
    
    // Stop here
    while (1) {
        stop(0xcccc);
    }
}


/*
 * HAL entry point
 */
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
