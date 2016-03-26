/*
 * System call dispatch
 */


#include "common/include/data.h"
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"
#include "kernel/include/syscall.h"


int dispatch_syscall(struct kernel_dispatch_info *disp_info)
{
    int resched = 0;
    struct thread *t = NULL;
    
    switch (disp_info->syscall.num) {
    case SYSCALL_KPUTS:
        t = create_thread(kernel_proc, (ulong)&kputs_worker_thread, (ulong)disp_info, -1, 0, 0);
        assert(t);
        break;
    default:
        break;
    }
    
    if (t) {
        run_thread(t);
        resched = 1;
    }
    
    return resched;
}
