#ifndef __ARCH_IA32_HAL_INCLUDE_LIB__
#define __ARCH_IA32_HAL_INCLUDE_LIB__


#include "common/include/data.h"
#include "common/include/bootparam.h"


#ifndef assert
#define assert(exp)
#endif

#ifndef warn
#define warn    kprintf
#endif


/*
 * Boot param
 */
extern struct boot_parameters *get_bootparam();


/*
 * IO ports
 */
extern void io_out32(ulong port, ulong value);
extern void io_out16(ulong port, ulong value);
extern void io_out8(ulong port, ulong value);

extern ulong io_in32(ulong port);
extern ulong io_in16(ulong port);
extern ulong io_in8(ulong port);


/*
 * Bit manipulation
 */
extern ulong get_bits(ulong value, int low, int high);
extern ulong round_up(ulong value);
extern ulong round_down(ulong value);


/*
 * String
 */
extern void memcpy(void *src, void *dest, size_t count);
extern void memset(void *src, int value, size_t size);
extern void memzero(void *src, size_t size);
extern int memcmp(void *src1, void *src2, size_t len);


/*
 * Misc
 */
extern void no_opt halt();


#endif
