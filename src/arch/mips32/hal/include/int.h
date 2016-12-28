#ifndef __ARCH_MIPS32_HAL_INCLUDE_INT__
#define __ARCH_MIPS32_HAL_INCLUDE_INT__


#include "common/include/data.h"


extern void init_int();
extern void tlb_refill_handler();
extern void cache_error_handler();
extern void general_except_handler();


extern void int_entry_template_begin();
extern void int_entry_template_end();


#endif
