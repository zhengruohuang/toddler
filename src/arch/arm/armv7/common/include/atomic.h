#ifndef __ARCH_ARMV7_COMMON_INCLUDE_ATOMIC__
#define __ARCH_ARMV7_COMMON_INCLUDE_ATOMIC__


#include "common/include/data.h"


/*
 * Comapre and swap
 */
static inline int atomic_cas(volatile void *target, ulong old_value, ulong new_value)
{
    return 0;
}

static inline int atomic_cas_uint(volatile void *target, unsigned int old_value, unsigned int new_value)
{
    return 0;
}


/*
 * Read and write
 */
static inline void atomic_write(volatile void *target, unsigned long value)
{
    *(unsigned long *)target = value;
}


/*
 * Fetch and add
 */
static inline void atomic_inc(volatile unsigned long *target)
{

}


/*
 * Memory barriers
 */
static inline void atomic_membar()
{
}

static inline void atomic_readbar()
{
}

static inline void atomic_writebar()
{
}


#endif
