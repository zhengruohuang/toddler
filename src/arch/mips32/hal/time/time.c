#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/int.h"
#include "hal/include/time.h"


static u64 cur_timestamp = 0;


void get_system_time(unsigned long *high, unsigned long *low)
{
//     kprintf("\tTimestamp: %p-%p\n",
//             (unsigned long)(cur_timestamp >> sizeof(unsigned long) * 8),
//             (unsigned long)cur_timestamp
//     );
    
    if (high) {
        *high = cur_timestamp >> (sizeof(unsigned long) * 8);
    }
    
    if (low) {
        *low = (unsigned long)cur_timestamp;
    }
}

// int time_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
// {
//     cur_timestamp++;
//     
//     kdi->interrupt.param0 = 0;
//     kdi->interrupt.param1 = 0;
//     kdi->interrupt.param2 = 0;
//     
//     return 0;
// }
// 
// void init_time()
// {
//     kprintf("Initializing system time\n");
//     
//     struct rtc_date_time dt;
//     cur_timestamp = read_timestamp(&dt);
//     
//     kprintf("\tCurrent Date Time: %d Century, %d-%d-%d %d:%d:%d\n",
//             dt.century,
//             dt.year, dt.month, dt.day,
//             dt.hour, dt.minute, dt.second
//     );
//     
//     kprintf("\tTimestamp: %p-%p\n",
//             (unsigned long)(cur_timestamp >> sizeof(unsigned long) * 8),
//             (unsigned long)cur_timestamp
//     );
// }
