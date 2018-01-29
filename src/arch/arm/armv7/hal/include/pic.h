#ifndef __ARCH_ARMV7_HAL_INCLUDE_PIC__
#define __ARCH_ARMV7_HAL_INCLUDE_PIC__


#include "common/include/data.h"
#include "hal/include/int.h"


/*
 * Timer
 */
extern int is_generic_timer_asserted();
extern void start_generic_timer();

extern void init_generic_timer();
extern void init_generic_timer_mp();

/*
 * Start working
 */
extern void start_working();
extern void start_working_mp();


#endif
