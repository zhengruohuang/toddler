#ifndef __HAL_INCLUDE_STRING__
#define __HAL_INCLUDE_STRING__


#include "common/include/data.h"


/*
 * String
 */
extern int strcmp(char *s1, char *s2);


/*
 * Memory
 */
extern void memcpy(void *dest, void *src, size_t count);
extern void memset(void *src, int value, size_t size);
extern void memzero(void *src, size_t size);
extern int memcmp(void *src1, void *src2, size_t len);


#endif
