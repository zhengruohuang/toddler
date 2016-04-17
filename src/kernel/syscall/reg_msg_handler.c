/*
 * System call worker thread - reg_msg_handler
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"
#include "kernel/include/ds.h"


void reg_msg_handler_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong msg_num = disp_info->syscall.param0;
    //ulong thread_entry = disp_info->syscall.param1;
    
    // Register the msg handler
    hashtable_insert(&p->msg_handlers, msg_num, NULL);
}
