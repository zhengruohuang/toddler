#include "common/include/data.h"
#include "common/include/task.h"
#include "hal/include/cpu.h"
#include "hal/include/task.h"
#include "hal/include/kernel.h"


void kernel_dispatch(struct kernel_dispatch_info *kdi)
{
    int *user_mode = get_per_cpu(int, cur_in_user_mode);
    
    // Save user mode flag
    int user_mode_save = *user_mode;
    
    // FIXME: Save and set ASID
    
    // Put us in kernel, so the TLB miss handler can correctly refill the entry
    *user_mode = 0;
    
    // Then call kernel dispatcher
    kernel->dispatch(*get_per_cpu(ulong, cur_running_sched_id), kdi);
    
    // Restore user mode flag
    *user_mode = user_mode_save;
}