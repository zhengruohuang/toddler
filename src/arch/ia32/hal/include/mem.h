#ifndef __ARCH_IA32_HAL_INCLUDE_MEM__
#define __ARCH_IA32_HAL_INCLUDE_MEM__


#include "common/include/data.h"


extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


#endif
