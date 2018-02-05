#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/periph.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/vector.h"
#include "hal/include/int.h"
#include "hal/include/kernel.h"
#include "hal/include/time.h"


void entry_func hal_entry(struct boot_parameters *boot_param)
{
    init_bootparam(boot_param);
    init_print();
    kprintf("We are in HAL!\n");
    
    // Init memory management
    init_pht();
    init_map();
    init_kalloc();
    
    // Init Periph
    init_periph();
    
    // Init OFW
//     init_ofw();
    
    // Init CPU
    init_cpuid();
    init_topo();
    init_mp();
    
    // Init interrupt
    init_int_vector();
    init_int();
    init_syscall();
    init_pagefault();
    
    // Init kernel
    init_kmem_zone();
    init_kernel();
    
    // Init timer
    init_decrementer();
    
    // Start working
    kprintf("Will start working!\n");
    start_working();
    
    // Should not reach here
//     halt();
    while (1);
}
