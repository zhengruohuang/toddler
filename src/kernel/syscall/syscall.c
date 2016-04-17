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
    
    // Put the thread to wait
    int is_in_wait = wait_thread(disp_info->thread);
    if (!is_in_wait) {
        return 1;
    }
    
    // Duplicate dispatch info
    struct kernel_dispatch_info *dup_disp_info = malloc(sizeof(struct kernel_dispatch_info));
    assert(dup_disp_info);
    memcpy(disp_info, dup_disp_info, sizeof(struct kernel_dispatch_info));
    
    // Do the actual dispatch
    switch (disp_info->syscall.num) {
    case SYSCALL_KPUTS:
        t = create_thread(kernel_proc, (ulong)&kputs_worker_thread, (ulong)dup_disp_info, -1, 0, 0);
        dup_disp_info->syscall.worker = t;
        assert(t);
        break;
    
    // IO Ports
    case SYSCALL_IO_IN:
        io_in_worker(disp_info);
        break;
    case SYSCALL_IO_OUT:
        io_out_worker(disp_info);
        break;
        
    // Message handler
    case SYSCALL_REG_MSG_HANDLER:
        reg_msg_handler_worker(disp_info);
        break;
        
    // Invalid syscall
    default:
        break;
    }
    
    // Take care of reschedule flag
    if (t) {
        run_thread(t);
        resched = 1;
    }
    
    return resched;
}
