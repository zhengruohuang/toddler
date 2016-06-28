#ifndef __KERNEL_INCLUDE_SYSCALL__
#define __KERNEL_INCLUDE_SYSCALL__


#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/syscall.h"
#include "kernel/include/proc.h"


/*
 * Dispatcher
 */
extern int dispatch_syscall(struct kernel_dispatch_info *disp_info);


/*
 * Syscall
 */
struct kernel_msg_handler_arg {
    struct thread *sender_thread;
    struct thread *handler_thread;
    msg_t *msg;
};

extern void init_syscall();
extern void init_ipc();

extern void kputs_worker_thread(ulong param);

extern void io_in_worker(struct kernel_dispatch_info *disp_info);
extern void io_out_worker(struct kernel_dispatch_info *disp_info);

extern void reg_msg_handler_worker(struct kernel_dispatch_info *disp_info);
extern void unreg_msg_handler_worker(struct kernel_dispatch_info *disp_info);
extern void send_worker(struct kernel_dispatch_info *disp_info);
extern void reply_worker(struct kernel_dispatch_info *disp_info);
extern void recv_worker_thread(ulong param);
extern void request_worker_thread(ulong param);
extern void respond_worker_thread(ulong param);

extern void reg_kapi_server_worker(struct kernel_dispatch_info *disp_info);
extern void unreg_kapi_server_worker(struct kernel_dispatch_info *disp_info);


#endif
