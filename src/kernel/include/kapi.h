#ifndef __KERNEL_INCLUDE_KAPI__
#define __KERNEL_INCLUDE_KAPI__


#include "common/include/data.h"
#include "common/include/task.h"
#include "kernel/include/syscall.h"
#include "kernel/include/ds.h"


extern hashtable_t kapi_servers;

extern void init_kapi();
extern asmlinkage void thread_exit_handler(struct kernel_msg_handler_arg *arg);

typedef asmlinkage void (*kernel_msg_handler_t)(struct kernel_msg_handler_arg *arg);


#endif
