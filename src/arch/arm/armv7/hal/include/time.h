#ifndef __ARCH_ARMV7_HAL_INCLUDE_TIME__
#define __ARCH_ARMV7_HAL_INCLUDE_TIME__


#include "common/include/data.h"
#include "hal/include/int.h"


/*
 * Tick
 */
// #define BLOCKED_DELAY_TEST_SEC  1
// #define TICK_FREQ               1
// 
// extern void change_tick(int freq);
// extern void init_tick();
// extern void blocked_delay(int ms);
// extern void init_blocked_delay();


/*
 * System time
 */
extern void get_system_time(unsigned long *high, unsigned long *low);
// extern int time_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi);
// extern void init_time();


#endif
