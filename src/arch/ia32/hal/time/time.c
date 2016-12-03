#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/time.h"


static int cuumu_days[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

void get_system_time(unsigned long *high, unsigned long *low)
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
    
    
    if (high) {
        *high = timestamp >> (sizeof(unsigned long) * 8);
    }
    
    if (low) {
        *low = (unsigned long)timestamp;
    }
}