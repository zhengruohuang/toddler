#ifndef __ARCH_IA32_HAL_INCLUDE_LIB__
#define __ARCH_IA32_HAL_INCLUDE_LIB__


#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/bit.h"
#include "hal/include/string.h"
#include "hal/include/debug.h"
#include "hal/include/bootparam.h"


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
 * Misc
 */
extern void no_opt halt();


#endif
