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


static int kernel_dispatch_salloc_id;


void init_syscall()
{
    kernel_dispatch_salloc_id = salloc_create(sizeof(struct kernel_dispatch_info), 0, 0, NULL, NULL);
    kprintf("\tSyscall kernel dispatch node salloc ID: %d\n", kernel_dispatch_salloc_id);
}

static struct kernel_dispatch_info *prepare_thread(struct kernel_dispatch_info *disp_info)
{
    // Put the thread to wait
    int is_in_wait = wait_thread(disp_info->thread);
    assert(is_in_wait);
    
    // Duplicate dispatch info
    struct kernel_dispatch_info *dup_disp_info = salloc(kernel_dispatch_salloc_id);
    assert(dup_disp_info);
    memcpy(disp_info, dup_disp_info, sizeof(struct kernel_dispatch_info));
    
    return dup_disp_info;
}

int dispatch_syscall(struct kernel_dispatch_info *disp_info)
{
    int resched = 0;
    struct thread *t = NULL;
    struct kernel_dispatch_info *dup_disp_info = NULL;
    
    // Do the actual dispatch
    switch (disp_info->syscall.num) {
    case SYSCALL_KPUTS:
        dup_disp_info = prepare_thread(disp_info);
        t = create_thread(kernel_proc, (ulong)&kputs_worker_thread, (ulong)dup_disp_info, -1, 0, 0);
        assert(t);
        break;
    
    // IO Ports
    case SYSCALL_IO_IN:
        io_in_worker(disp_info);
        break;
    case SYSCALL_IO_OUT:
        io_out_worker(disp_info);
        break;
        
    // IPC
    case SYSCALL_REG_MSG_HANDLER:
        kprintf("syscall reg msg handler\n");
        reg_msg_handler_worker(disp_info);
        break;
    case SYSCALL_UNREG_MSG_HANDLER:
        unreg_msg_handler_worker(disp_info);
        break;
    case SYSCALL_SEND:
        send_worker(disp_info);
        break;
    case SYSCALL_REPLY:
        reply_worker(disp_info);
        break;
    case SYSCALL_RECV:
        dup_disp_info = prepare_thread(disp_info);
        t = create_thread(kernel_proc, (ulong)&recv_worker_thread, (ulong)dup_disp_info, -1, 0, 0);
        assert(t);
        break;
    case SYSCALL_REQUEST:
        kprintf("syscall request\n");
        dup_disp_info = prepare_thread(disp_info);
        t = create_thread(kernel_proc, (ulong)&request_worker_thread, (ulong)dup_disp_info, -1, 0, 0);
        assert(t);
        break;
    case SYSCALL_RESPOND:
        kprintf("syscall respond\n");
        dup_disp_info = prepare_thread(disp_info);
        t = create_thread(kernel_proc, (ulong)&respond_worker_thread, (ulong)dup_disp_info, -1, 0, 0);
        assert(t);
        break;
        
    // KAPI
    case SYSCALL_REG_KAPI_SERVER:
        kprintf("syscall reg kapi server\n");
        reg_kapi_server_worker(disp_info);
        break;
    case SYSCALL_UNREG_KAPI_SERVER:
        unreg_kapi_server_worker(disp_info);
        break;
        
    // Invalid syscall
    default:
        break;
    }
    
    // Take care of reschedule flag
    if (t) {
        dup_disp_info->syscall.worker = t;
        run_thread(t);
        resched = 1;
    }
    
    return resched;
}
