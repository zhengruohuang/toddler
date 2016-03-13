#ifndef __ARCH_IA32_HAL_INCLUDE_KERNEL__
#define __ARCH_IA32_HAL_INCLUDE_KERNEL__


#include "common/include/data.h"

#ifndef __HAL__
#define __HAL__
#endif
#include "kernel/include/hal.h"


/*
 * Wrappers
 */
extern void asmlinkage wrap_kernel_map(ulong addr, size_t size);
extern int asmlinkage wrap_user_map(ulong page_dir, ulong vaddr, ulong paddr, size_t size, int exec, int write, int cacheable);
extern void asmlinkage wrap_halt();


/*
 * Mem zone
 */
extern ulong paddr_space_end;

extern int asmlinkage get_next_mem_zone(struct kernel_mem_zone *cur);
extern void init_kmem_zone();


/*
 * Init kernel
 */
extern void init_kernel();


#endif
