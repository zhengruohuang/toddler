#include "common/include/data.h"
#include "common/include/reg.h"
#include "hal/include/int.h"
#include "hal/include/pic.h"
#include "hal/include/periph.h"


void start_working()
{
    // Enable external interrupt controller
    struct cp0_status sr;
    read_cp0_status(sr.value);
    
    sr.im2 = 1;
    write_cp0_status(sr.value);
    
    // Enable 8259
    i8259_start();
    
    // Enable timer counter interrupt
    enable_local_timer_interrupt();
    
    // Enable global interrupt
    enable_local_int();
}
