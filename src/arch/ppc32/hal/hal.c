#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/periph.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"


void no_opt hal_entry(struct boot_parameters *boot_param)
{
    init_bootparam(boot_param);
    init_print();
    kprintf("We are in HAL!\n");
    
    // Init memory management
    init_pht();
    init_map();
    init_kalloc();
    
    // Init CPU
    init_cpuid();
    init_topo();
    init_mp();
    
    // Init task
    
    // Init interrupt
    init_int();
    
    // Init timer
    
    // Init kernel
    
    halt();
    while (1);
}
