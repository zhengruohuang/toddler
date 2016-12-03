#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/time.h"


void get_system_time(unsigned long *high, unsigned long *low)
{
    if (high) {
        *high = 0x100;
    }
    
    if (low) {
        *low = 0x200;
    }
}