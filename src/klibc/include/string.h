#ifndef __KLIBC_INCLUDE_STRING__
#define __KLIBC_INCLUDE_STRING__


/*
 * String
 */
extern size_t strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern void strcpy(char *dest, const char *src);
extern char *strdup(const char *str);


/*
 * Memory
 */
extern void memcpy(void *dest, const void *src, size_t count);
extern void memset(void *src, int value, size_t size);
extern void memzero(void *src, size_t size);
extern int memcmp(const void *src1, const void *src2, size_t len);


#endif
