#include "common/include/data.h"
#include "hal/include/int.h"
#include "hal/include/pic.h"


void start_working()
{
    // Enable timer counter interrupt
    enable_local_timer_interrupt();
    
    // Enable global interrupt
    enable_local_int();
}
