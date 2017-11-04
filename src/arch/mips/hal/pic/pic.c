#include "common/include/data.h"
#include "hal/include/int.h"
#include "hal/include/pic.h"
#include "hal/include/periph.h"


void start_working()
{
    // Enable external interrupt controller
    u32 sr = 0;
    __asm__ __volatile__ (
        "mfc0   %0, $12;"
        : "=r" (sr)
        :
    );
    sr |= 0x1 << 10;
    __asm__ __volatile__ (
        "mtc0   %0, $12;"
        :
        : "r" (sr)
    );
    i8259_start();
    
    // Enable timer counter interrupt
    enable_local_timer_interrupt();
    
    // Enable global interrupt
    enable_local_int();
}
