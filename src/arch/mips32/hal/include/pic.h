#ifndef __ARCH_MIPS32_HAL_INCLUDE_PIC__
#define __ARCH_MIPS32_HAL_INCLUDE_PIC__


#include "common/include/data.h"
#include "common/include/task.h"
#include "hal/include/int.h"


extern void start_working();

extern int int_handler_local_timer(struct int_context *context, struct kernel_dispatch_info *kdi);
extern void enable_local_timer_interrupt();
extern void init_local_timer();


#endif
