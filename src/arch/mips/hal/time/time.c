#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/int.h"
#include "hal/include/periph.h"
#include "hal/include/time.h"


static u64 cur_timestamp = 0;
static int cuumu_days[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};


static u64 read_timestamp(struct rtc_date_time *out)
{
    struct rtc_date_time dt;
    read_rtc(&dt);
    
    u64 timestamp = 0;
    
    // Days before this year
    timestamp += dt.year * 365
        + dt.year / 4
        - dt.year / 100
        + dt.year / 400;
    
    // Days before this month
    timestamp += cuumu_days[dt.month - 1];
    
    // Dyas in this month
    timestamp += dt.day;
    
    // Leap year
    int is_this_year_leap = (dt.year % 4 == 0) && (dt.year % 100 || dt.year % 400 == 0);
    if (dt.month > 2 && is_this_year_leap) {
        timestamp += 1;
    }
    
    // Calcuate seconds
    timestamp *= 24 * 3600;
    timestamp += dt.hour * 3600;
    timestamp += dt.minute * 60;
    timestamp += dt.second;
    
    if (out) {
        *out = dt;
    }
    
    return timestamp;
}


void get_system_time(unsigned long *high, unsigned long *low)
{
//     kprintf("\tTimestamp: %p-%p\n",
//             (unsigned long)(cur_timestamp >> sizeof(unsigned long) * 8),
//             (unsigned long)cur_timestamp
//     );
    
    if (high) {
#if (ARCH_WIDTH == 64)
        *high = 0;
#else
        *high = (unsigned long)(cur_timestamp >> 32);
#endif
    }
    
    if (low) {
        *low = (unsigned long)cur_timestamp;
    }
}

int time_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    cur_timestamp++;
    
    kdi->interrupt.param0 = 0;
    kdi->interrupt.param1 = 0;
    kdi->interrupt.param2 = 0;
    
//     kprintf("Time!\n");
    
    i8259_eoi(0);
    
    return INT_HANDLE_TYPE_TAKEOVER;
}

void init_time()
{
    kprintf("Initializing system time\n");
    
    struct rtc_date_time dt;
    cur_timestamp = read_timestamp(&dt);
    
    kprintf("\tCurrent Date Time: %d Century, %d-%d-%d %d:%d:%d\n",
            dt.century,
            dt.year, dt.month, dt.day,
            dt.hour, dt.minute, dt.second
    );
    
    ulong high = 0, low = 0;
    get_system_time(&high, &low);
    kprintf("\tTimestamp: %p-%p\n", (void *)high, (void *)low);
    
    // Register interrupt handler
    set_int_vector(INT_VECTOR_EXTERNAL_BASE + 0, time_interrupt_handler);
}
