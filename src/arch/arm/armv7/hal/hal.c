#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/bootparam.h"
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


extern void init_periph();


static void init_stack()
{
}


/*
 * Secondary processors
 */
static void mp_entry()
{
    stop(0xaaaa);
}

#include "common/include/memory.h"

struct l1table;
extern struct l1table *kernel_l1table;

static void bringup_mp()
{
    ulong vaddr = (ulong)mp_entry;
    ulong paddr = get_paddr(ADDR_TO_PFN((ulong)kernel_l1table), vaddr);
    
    int num = 1;
    for (num = 1; num < 4; num++) {
        *(volatile u32 *)(ulong)(0x4000008C + 0x10 * num) = (u32)paddr;
    }
}


/*
 * Primary processor
 */
static void start_hal(struct boot_parameters *boot_param)
{
    // Make bootparam globally accessible
    init_bootparam(boot_param);
    
    // Should be safe to swich stack at this point
    init_stack();
    
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
    
    // Init timer
    init_generic_timer();
    
    // Init kernel
    init_kmem_zone();
    init_kernel();
    
    // Init MP
//     bringup_mp();
//     while (1);
    
    // Start working
    start_working();
    
//     // Init memory management
//     init_pht();
//     init_map();
//     init_kalloc();
//     
//     // Init OFW
//     init_ofw();
//     
//     // Init CPU
//     init_cpuid();
//     init_topo();
//     init_mp();
//     
//     // Init interrupt
//     init_int_vector();
//     init_int();
//     init_syscall();
//     init_pagefault();
//     
//     // Init kernel
//     init_kmem_zone();
//     init_kernel();
//     
//     // Init timer
//     init_decrementer();
//     
//     // Start working
//     start_working();
    
    // Stop here
    kprintf("Initialized!\n");
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
