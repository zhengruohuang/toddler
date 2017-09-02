#ifndef __ARCH_PPC32_HAL_INCLUDE_TIME__
#define __ARCH_PPC32_HAL_INCLUDE_TIME__


#include "common/include/data.h"
#include "hal/include/int.h"


/*
 * Decrementer
 */
extern void init_decrementer();


/*
 * System time
 */
extern void get_system_time(unsigned long *high, unsigned long *low);
// extern int time_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi);
// extern void init_time();


#endif
