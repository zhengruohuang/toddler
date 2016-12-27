#ifndef __KERNEL_INCLUDE_LIB__
#define __KERNEL_INCLUDE_LIB__


#include "common/include/data.h"


/*
 * Str
 */
extern size_t strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern void strcpy(char *dest, const char *src);
extern char *strdup(const char *s);


/*
 * Mem
 */
extern void memcpy(void *dest, void *src, size_t count);
extern void memset(void *src, int value, size_t size);
extern void memzero(void *src, size_t size);
extern int memcmp(void *src1, void *src2, size_t len);


#endif
