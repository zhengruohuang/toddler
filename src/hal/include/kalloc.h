#ifndef __HAL_INCLUDE_KALLOC__
#define __HAL_INCLUDE_KALLOC__


#include "common/include/data.h"


/*
 * HAL memory pool
 */
extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


#endif

