#ifndef __KLIBC_INCLUDE_STDLIB__
#define __KLIBC_INCLUDE_STDLIB__


#include "common/include/data.h"


extern void *malloc(size_t size);
extern void *calloc(int count, size_t size);
extern void free(void *size);


#endif
