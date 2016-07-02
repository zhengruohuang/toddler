#ifndef __KLIBC_INCLUDE_STRING__
#define __KLIBC_INCLUDE_STRING__


/*
 * String
 */
extern size_t strlen(char *s);
extern int strcmp(char *s1, char *s2);
extern void strcpy(char *dest, char *src);


/*
 * Memory
 */
extern void memcpy(void *dest, void *src, size_t count);
extern void memset(void *src, int value, size_t size);
extern void memzero(void *src, size_t size);
extern int memcmp(void *src1, void *src2, size_t len);


#endif
