#ifndef __ARCH_MIPS32_HAL_INCLUDE_KERNEL__
#define __ARCH_MIPS32_HAL_INCLUDE_KERNEL__


#include "common/include/data.h"
#include "common/include/kexport.h"


/*
 * Memory zone
 */
extern ulong paddr_space_end;

extern int get_next_mem_zone(struct kernel_mem_zone *cur);
extern void init_kmem_zone();


/*
 * General kernel
 */
extern struct kernel_exports *kernel;

extern void init_kernel();
extern void kernel_dispatch(struct kernel_dispatch_info *kdi);


#endif
