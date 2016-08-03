#ifndef __KERNEL_INCLUDE_KAPI__
#define __KERNEL_INCLUDE_KAPI__


#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/syscall.h"
#include "kernel/include/syscall.h"
#include "kernel/include/proc.h"
#include "kernel/include/ds.h"


/*
 * KAPI
 */
typedef asmlinkage void (*kernel_msg_handler_t)(struct kernel_msg_handler_arg *arg);

extern hashtable_t kapi_servers;

extern void init_kapi();
extern msg_t *create_response_msg(struct thread *t);
extern void set_msg_param_value(msg_t *m, unsigned long value);


/*
 * Thread
 */
extern asmlinkage void thread_create_handler(struct kernel_msg_handler_arg *arg);
extern asmlinkage void thread_exit_handler(struct kernel_msg_handler_arg *arg);


/*
 * Interrupt
 */
extern asmlinkage void reg_interrupt_handler(struct kernel_msg_handler_arg *arg);
extern asmlinkage void unreg_interrupt_handler(struct kernel_msg_handler_arg *arg);


/*
 * Process
 */
extern asmlinkage void process_started_handler(struct kernel_msg_handler_arg *arg);
extern asmlinkage void process_exit_handler(struct kernel_msg_handler_arg *arg);


/*
 * Heap
 */
extern asmlinkage void set_heap_end_handler(struct kernel_msg_handler_arg *arg);
extern asmlinkage void get_heap_end_handler(struct kernel_msg_handler_arg *arg);
extern asmlinkage void grow_heap_handler(struct kernel_msg_handler_arg *arg);
extern asmlinkage void shrink_heap_handler(struct kernel_msg_handler_arg *arg);


#endif
