/*
 * System call worker thread - time
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/syscall.h"


void time_worker(struct kernel_dispatch_info *disp_info)
{
    // Get time
    unsigned long high = 0;
    unsigned long low = 0;
    hal->time(&high, &low);
    
    set_syscall_return(disp_info->thread, high, low);
}
