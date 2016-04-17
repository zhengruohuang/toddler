#ifndef __KERNEL_INCLUDE_SYSCALL__
#define __KERNEL_INCLUDE_SYSCALL__


#include "common/include/data.h"
#include "common/include/task.h"


/*
 * Dispatcher
 */
extern int dispatch_syscall(struct kernel_dispatch_info *disp_info);


/*
 * Syscall workers
 */
extern void kputs_worker_thread(ulong param);

extern void io_in_worker(struct kernel_dispatch_info *disp_info);
extern void io_out_worker(struct kernel_dispatch_info *disp_info);

extern void reg_handler_worker(struct kernel_dispatch_info *disp_info);
extern void rel_handler_worker(struct kernel_dispatch_info *disp_info);
extern void send_worker(struct kernel_dispatch_info *disp_info);
extern void reply_worker(struct kernel_dispatch_info *disp_info);
extern void recv_worker_thread(ulong param);
extern void request_worker_thread(ulong param);
extern void respond_worker_thread(ulong param);


#endif
