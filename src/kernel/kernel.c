#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/coreimg.h"
#include "kernel/include/ds.h"
#include "kernel/include/syscall.h"
#include "kernel/include/kapi.h"


struct hal_exports *hal;
spinlock_t kprintf_lock = { {0} };


/*
 * Kernel function wrappers
 */
static ulong wrap_palloc_tag(int count, int tag)
{
    return palloc_tag(count, tag);
}

static ulong wrap_palloc(int count)
{
    return palloc(count);
}

static int wrap_pfree(ulong pfn)
{
    return pfree(pfn);
}


/*
 * Dispatch
 */
static void dispatch(ulong sched_id, struct kernel_dispatch_info *disp_info)
{
    struct sched *s = get_sched(sched_id);
    
    // Deschedule current thread
    int need_dispatch = desched(sched_id, disp_info->context);
    
    // TLB shootdown
    service_tlb_shootdown();
    
    if (need_dispatch) {
        // Need a private stack for MIPS kernel dispatch as a TLB miss handler can overwrite the data in kernel stack
//         kprintf("dispatch kernel @ %x, type: %x, syscall num: %x\n", disp_info, disp_info->dispatch_type, disp_info->syscall.num);
        
        // Fill in the dispatch infio
        disp_info->proc = s->proc;
        disp_info->thread = s->thread;
        disp_info->sched = s;
        
        switch (disp_info->dispatch_type) {
        case kdisp_syscall:
//             kprintf("dispatch syscall, num: %x\n", disp_info->syscall.num);
            dispatch_syscall(disp_info);
            break;
        case kdisp_interrupt:
            dispatch_interrupt(disp_info);
            break;
        default:
            break;
        }
        
        resched(sched_id);
    }
    
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
    hal->kernel->pfree = wrap_pfree;
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
    
    // Init built-in data structions
    //init_list();
    init_hashtable();
    
    // Init URS
//     init_urs();
    
    // Init process mgr
    init_asid();
    init_sched();
    init_process();
    init_thread();
    init_dalloc();
    init_tlb_mgmt();
    
    // Init dispatch, syscall, interrupt, and exception
    init_dispatch();
    init_interrupt();
    
    // Init IPC and KAPI
    init_ipc();
    init_kapi();
    
    // Init namespace dispatcher
    
    // Init core image
    init_coreimg();
//     init_coreimgfs();
    
    // Kernel exports
    init_kexp();
    
    // Load user programs
    start_user();
}
