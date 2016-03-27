/*
 * System call dispatch
 */


#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/syscall.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"
#include "kernel/include/syscall.h"


int dispatch_syscall(struct kernel_dispatch_info *disp_info)
{
    int resched = 0;
    struct thread *t = NULL;
    
    // Duplicate dispatch info
    struct kernel_dispatch_info *dup_disp_info = malloc(sizeof(struct kernel_dispatch_info));
    assert(dup_disp_info);
    memcpy(disp_info, dup_disp_info, sizeof(struct kernel_dispatch_info));
    
    switch (disp_info->syscall.num) {
    case SYSCALL_KPUTS:
        wait_thread(dup_disp_info->thread);
        t = create_thread(kernel_proc, (ulong)&kputs_worker_thread, (ulong)dup_disp_info, -1, 0, 0);
        dup_disp_info->syscall.worker = t;
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
