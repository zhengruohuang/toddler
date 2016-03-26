/*
 * System call worker thread - kputs
 */
#include <common/include/task.h>


void kputs_worker_thread(ulong param)
{
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    
    ulong start_vaddr = disp_info->syscall.param;
}
