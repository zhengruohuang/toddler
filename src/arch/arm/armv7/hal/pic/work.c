#include "common/include/data.h"
#include "common/include/reg.h"
#include "hal/include/int.h"
#include "hal/include/pic.h"
#include "hal/include/periph.h"


void start_working()
{
    // Start the generic timer
    start_generic_timer();
    
    // Start periph
    start_periph();
    
    // Enable local interrupt
    enable_local_int();
}

void start_working_mp()
{
    // Start the generic timer
    start_generic_timer();
    
    // Enable local interrupt
    enable_local_int();
}
