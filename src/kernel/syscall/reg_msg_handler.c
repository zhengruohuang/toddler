/*
 * System call worker thread - reg_msg_handler
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"


void reg_msg_handler_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    
}
