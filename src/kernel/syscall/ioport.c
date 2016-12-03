/*
 * System call worker thread - read and write IO ports
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/syscall.h"


void io_in_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong addr = disp_info->syscall.param0;
    ulong size = disp_info->syscall.param1;
    ulong result = 0;
    
    switch (p->type) {
    case process_kernel:
    case process_system:
    case process_driver:
    case process_emulate:
        result = hal->io_in(addr, size);
        break;
    default:
        // Crash the process
        break;
    }
    
//     disp_info->syscall.return0 = result;
    set_syscall_return(disp_info->thread, result, 0);
}

void io_out_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong addr = disp_info->syscall.param0;
    ulong size = disp_info->syscall.param1;
    ulong value = disp_info->syscall.param2;
    
    switch (p->type) {
    case process_kernel:
    case process_system:
    case process_driver:
    case process_emulate:
        hal->io_out(addr, size, value);
        break;
    default:
        // Crash the process
        break;
    }
}
