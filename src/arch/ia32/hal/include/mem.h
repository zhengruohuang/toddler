#ifndef __ARCH_IA32_HAL_INCLUDE_MEM__
#define __ARCH_IA32_HAL_INCLUDE_MEM__


#include "common/include/data.h"


extern void kernel_indirect_map(ulong vaddr, ulong paddr, int disable_cache);
extern void kernel_indirect_map_array(ulong vaddr, ulong paddr, size_t size, int disable_cache);
extern void kernel_direct_map(ulong addr, int disable_cache);
extern void kernel_direct_map_array(ulong addr, size_t size, int disable_cache);


extern ulong palloc(int count);
extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


#endif
