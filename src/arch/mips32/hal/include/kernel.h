#ifndef __ARCH_MIPS32_HAL_INCLUDE_KERNEL__
#define __ARCH_MIPS32_HAL_INCLUDE_KERNEL__


#include "common/include/data.h"
#include "common/include/task.h"

#ifndef __HAL__
#define __HAL__
#endif
#include "kernel/include/hal.h"


/*
 * Init kernel
 */
extern struct kernel_exports *kernel;

extern void init_kernel();
extern void kernel_dispatch(struct kernel_dispatch_info *kdi);


#endif
