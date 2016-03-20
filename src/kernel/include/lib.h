#ifndef __KERNEL_INCLUDE_LIB__
#define __KERNEL_INCLUDE_LIB__


#include "common/include/data.h"


/*
 * Str
 */
extern size_t strlen(char *s);
extern int strcmp(char *s1, char *s2);
extern void strcpy(char *dest, char *src);
extern char *strdup(char *s);


/*
 * Mem
 */
extern void memcpy(void *src, void *dest, size_t count);
extern void memset(void *src, int value, size_t size);
extern void memzero(void *src, size_t size);
extern int memcmp(void *src1, void *src2, size_t len);


#endif
