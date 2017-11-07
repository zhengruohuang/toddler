#ifndef __LOADER_LIB_H__
#define __LOADER_LIB_H__


#include "common/include/data.h"


/*
 * Alignment
 */
#define ALIGN_UP(s, a)      (((s) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(s, a)    ((s) & ~((a) - 1))


/*
 * BSS
 */
extern void init_bss();


/*
 * Endian
 */
extern u64 swap_endian64(u64 val);
extern u32 swap_endian32(u32 val);
extern u16 swap_endian16(u16 val);
extern ulong swap_endian(ulong val);


/*
 * Math
 */
extern void div_u32(u32 dividend, u32 divisor, u32 *quo_out, u32 *rem_out);
extern void div_s32(s32 dividend, s32 divisor, s32 *quo_out, s32 *rem_out);
extern void div_int(int dividend, int divisor, int *quo_out, int *rem_out);
extern void div_uint(uint dividend, uint divisor, uint *quo_out, uint *rem_out);


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
