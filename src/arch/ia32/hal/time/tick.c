#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/time.h"


void change_tick(int freq)
{
    pit_gen_tick(TICK_FREQ);
}

void init_tick()
{
    pit_gen_tick(TICK_FREQ);
}

void blocked_delay(int ms)
{
    int loop = ms / PIT_DELAY_MAX_MS;
    int last = ms % PIT_DELAY_MAX_MS;
    
    int i;
    for (i = 0; i < loop; i++) {
        pit_delay(PIT_DELAY_MAX_MS);
    }
    if (last) {
        pit_delay(last);
    }
}

void init_blocked_delay()
{
    kprintf("Testing blocked delay, delay for %d seconds ", BLOCKED_DELAY_TEST_SEC);
    
    int i;
    for (i = 0; i < BLOCKED_DELAY_TEST_SEC; i++) {
        blocked_delay(1000);
        kprintf(".");
    }
    
    kprintf(" Done!\n");
}
