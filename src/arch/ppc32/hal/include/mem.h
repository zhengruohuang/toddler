#ifndef __ARCH_PPC32_HAL_INCLUDE_MEM__
#define __ARCH_PPC32_HAL_INCLUDE_MEM__


#include "common/include/data.h"


/*
 * PHT
 */
extern void init_pht();

extern void fill_kernel_pht(ulong vstart, ulong len, int io, int persist);
extern void evict_kernel_pht(ulong vstart, ulong len);


/*
 * KAlloc
 */
extern ulong palloc(int count);
extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


/*
 * Mapping
 */
extern void kernel_map_per_cpu_area(ulong vaddr, ulong paddr);
extern void init_map();


#endif
