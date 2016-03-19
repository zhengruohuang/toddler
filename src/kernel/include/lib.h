#ifndef __KERNEL_INCLUDE_LIB__
#define __KERNEL_INCLUDE_LIB__


#include "common/include/data.h"


/*
 * String
 */
extern void memcpy(void *src, void *dest, size_t count);
extern void memset(void *src, int value, size_t size);
extern void memzero(void *src, size_t size);
extern int memcmp(void *src1, void *src2, size_t len);


#endif
