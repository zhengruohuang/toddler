#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/percpu.h"
#include "hal/include/kernel.h"


void kernel_dispatch(struct kernel_dispatch_info *kdi)
{
    int *user_mode = get_per_cpu(int, cur_in_user_mode);
    
    // Save user mode flag
    int user_mode_save = *user_mode;
    
//     // Save ASID in TLB EntryHi
//     u32 hi = 0;
//     u32 asid = 0;
//     __asm__ __volatile__ (
//         "mfc0   %0, $10;"
//         : "=r" (hi)
//         :
//     );
//     asid = hi & 0xff;
//     hi &= ~0xff;
//     __asm__ __volatile__ (
//         "mtc0   %0, $10;"
//         :
//         : "r" (hi)
//     );
    
    // Put us in kernel, so the TLB miss handler can correctly refill the entry
    *user_mode = 0;
    
    // Then call kernel dispatcher
    kernel->dispatch(*get_per_cpu(ulong, cur_running_sched_id), kdi);
    
    // Restore user mode flag
    *user_mode = user_mode_save;
    
//     // Restore ASID
//     hi |= asid & 0xff;
//     __asm__ __volatile__ (
//         "mtc0   %0, $10;"
//         :
//         : "r" (hi)
//     );
}
