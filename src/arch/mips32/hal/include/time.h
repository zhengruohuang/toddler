#ifndef __ARCH_MIPS32_HAL_INCLUDE_TIME__
#define __ARCH_MIPS32_HAL_INCLUDE_TIME__


#include "common/include/data.h"


/*
 * System time
 */
extern void get_system_time(unsigned long *high, unsigned long *low);
// extern int time_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi);
// extern void init_time();


#endif
