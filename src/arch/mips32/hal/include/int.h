#ifndef __ARCH_MIPS32_HAL_INCLUDE_INT__
#define __ARCH_MIPS32_HAL_INCLUDE_INT__


#include "common/include/data.h"


/*
 * Generic
 */
extern void init_int();

extern void tlb_refill_handler(struct context *context);
extern void cache_error_handler(struct context *context);
extern void general_except_handler(struct context *context);


/*
 * Entry
 */
extern void int_entry_wrapper_begin();
extern void int_entry_wrapper_end();


#endif
