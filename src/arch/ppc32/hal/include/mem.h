#ifndef __ARCH_PPC32_HAL_INCLUDE_MEM__
#define __ARCH_PPC32_HAL_INCLUDE_MEM__


#include "common/include/data.h"


/*
 * PHT
 */
extern void init_pht();


/*
 * KAlloc
 */
extern ulong palloc(int count);
extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


#endif
