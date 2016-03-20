#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/coreimg.h"


struct hal_exports *hal;


/*
 * Kernel function wrappers
 */
ulong asmlinkage wrap_palloc_tag(int count, int tag)
{
    return palloc_tag(count, tag);
}

ulong asmlinkage wrap_palloc(int count)
{
    return palloc(count);
}


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
    hal->kernel->palloc_tag = wrap_palloc_tag;
    hal->kernel->palloc = wrap_palloc;
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
    init_coreimg();
    
    // Kernel exports
    init_kexp();
    
    // Load user programs
    start_user();
}
