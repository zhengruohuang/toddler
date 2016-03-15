#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


struct hal_exports *hal;


/*
 * This is the entry point of Kernel
 */
void asmlinkage _start(struct hal_exports *hal_exp)
{
    hal = hal_exp;
    kprintf("We are in the kernel!\n");
    
    // Init PFN DB
    init_pfndb();
    
    // Init page allocator
    init_palloc();
    test_palloc();
    
    // Init kernel malloc
    init_salloc();
    init_malloc();
    test_malloc();
    
    // Init process mgr
    init_process();
    init_thread();
    
    // Init namespace dispatcher
    
    // Init core image
    
    // Load first user program
}
