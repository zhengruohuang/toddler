#ifndef __ARCH_IA32_HAL_INCLUDE_LIB__
#define __ARCH_IA32_HAL_INCLUDE_LIB__


#include "common/include/data.h"
#include "common/include/bootparam.h"


extern struct boot_parameters *get_bootparam();

extern void io_out32(ulong port, ulong value);
extern void io_out16(ulong port, ulong value);
extern void io_out8(ulong port, ulong value);

extern ulong io_in32(ulong port);
extern ulong io_in16(ulong port);
extern ulong io_in8(ulong port);

extern void no_opt halt();


#endif
