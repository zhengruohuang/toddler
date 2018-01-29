#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/reg.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/kernel.h"


void kernel_dispatch(struct kernel_dispatch_info *kdi)
{
//     int *user_mode = get_per_cpu(int, cur_in_user_mode);
//     
//     // Save user mode flag
//     int user_mode_save = *user_mode;
//     
//     // Save ASID in TLB EntryHi
//     struct cp0_entry_hi old_hi, hi;
//     read_cp0_entry_hi(old_hi.value);
//     
//     // Set ASID to 0
//     hi.value = old_hi.value;
//     hi.asid = 0;
//     write_cp0_entry_hi(hi.value);
//     
//     // Put us in kernel, so the TLB miss handler can correctly refill the entry
//     *user_mode = 0;
    
    // FIXME
    // Maybe interrupts should not be enabled here at all?
    // We are still using the context-saving stack!
    // Enable interrupts
//     enable_local_int();
    
    // Then call kernel dispatcher
    kernel->dispatch(*get_per_cpu(ulong, cur_running_sched_id), kdi);
    
    // Restore user mode flag
//     *user_mode = user_mode_save;
    
//     // Restore ASID
//     write_cp0_entry_hi(old_hi.value);
}
