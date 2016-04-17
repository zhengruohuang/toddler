/*
 * System call worker thread - read and write IO ports
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"


void io_in_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
//     struct process *p = disp_info->proc;
//     ulong addr = disp_info->syscall.param0;
//     ulong size = disp_info->syscall.param1;
}

void io_out_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
//     struct process *p = disp_info->proc;
//     ulong addr = disp_info->syscall.param0;
//     ulong size = disp_info->syscall.param1;
//     ulong value = disp_info->syscall.param2;
}
