#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


struct hal_exports *hal;


/*
 * Dispatch
 */
static void asmlinkage dispatch(ulong sched_id, struct kernel_dispatch_info *int_info)
{
    //kprintf("Dispatch\n");
    desched(sched_id, int_info->context);
    sched();
}

/*
 * Kernel exports
 */
static void init_kexp()
{
    hal->kernel->dispatch = dispatch;
}

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
    init_sched();
    init_process();
    init_thread();
    
    // Init namespace dispatcher
    
    // Init core image
    
    // Load first user program
    
    // Kernel exports
    init_kexp();
}
