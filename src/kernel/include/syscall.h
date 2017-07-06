#ifndef __KERNEL_INCLUDE_SYSCALL__
#define __KERNEL_INCLUDE_SYSCALL__


#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/proc.h"
#include "common/include/syscall.h"
#include "kernel/include/proc.h"


/*
 * Dispatch
 */
extern void init_dispatch();
extern void set_syscall_return(struct thread *t, unsigned long return0, unsigned long return1);
extern int dispatch_syscall(struct kernel_dispatch_info *disp_info);
extern int dispatch_interrupt(struct kernel_dispatch_info *disp_info);


/*
 * Syscall
 */
struct kernel_msg_handler_arg {
    struct thread *sender_thread;
    struct thread *handler_thread;
    msg_t *msg;
};

extern void init_ipc();

extern void kputs_worker_thread(ulong param);

extern void time_worker(struct kernel_dispatch_info *disp_info);

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

extern no_opt struct thread_control_block *ksys_get_tcb();
extern int ksys_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2);
extern msg_t *ksys_msg();
extern msg_t *ksys_request();


/*
 * Interrupt
 */
extern void init_interrupt();
extern void reg_interrupt(struct process *p, unsigned long irq, unsigned long thread_entry);
extern void unreg_interrupt(struct process *p, unsigned long irq);
extern void interrupt_worker(struct kernel_dispatch_info *disp_info);


#endif
