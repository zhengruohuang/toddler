#ifndef __KERNEL_INCLUDE_KAPI__
#define __KERNEL_INCLUDE_KAPI__


#include "common/include/data.h"
#include "common/include/task.h"
#include "kernel/include/syscall.h"
#include "kernel/include/ds.h"


/*
 * KAPI
 */
typedef asmlinkage void (*kernel_msg_handler_t)(struct kernel_msg_handler_arg *arg);

extern hashtable_t kapi_servers;

extern void init_kapi();


/*
 * Thread
 */
extern asmlinkage void thread_exit_handler(struct kernel_msg_handler_arg *arg);


/*
 * Interrupt
 */
extern asmlinkage void reg_interrupt_handler(struct kernel_msg_handler_arg *arg);
extern asmlinkage void unreg_interrupt_handler(struct kernel_msg_handler_arg *arg);




#endif
