#include "common/include/data.h"
#include "loader/include/periph/bcm2835.h"


#define BCM2835_TIMER_BASE  (0x3000)


struct bcm2835_timer {
    volatile u32 control_status;
    volatile u32 counter_lo;
    volatile u32 counter_hi;
    volatile u32 compare0;
    volatile u32 compare1;
    volatile u32 compare2;
    volatile u32 compare3;
};


static volatile struct bcm2835_timer *timer;


void init_bcm2835_timer(ulong bcm2835_base)
{
    timer = (void *)(bcm2835_base + BCM2835_TIMER_BASE);
}

void bcm2835_delay(int d)
{
    const u32 clo = timer->counter_lo;
    u32 c = clo;
    do {
        c = timer->counter_lo;
    } while (c && c - clo < (u32)d);
}

u64 bcm2835_timer_read()
{
    u32 hi = timer->counter_hi;
    u32 lo = timer->counter_lo;
    while (timer->counter_hi != hi) {
        hi = timer->counter_hi;
    }
    
    u64 t = ((u64)hi << 32) | (u64)lo;
    return t;
}
