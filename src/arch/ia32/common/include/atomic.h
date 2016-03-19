#ifndef __ARCH_IA32_COMMON_INCLUDE_ATOMIC__
#define __ARCH_IA32_COMMON_INCLUDE_ATOMIC__


#include "common/include/data.h"


/*
 * Comapre and swap
 */
static inline int atomic_cas(volatile void *target, ulong old_value, ulong new_value)
{
    ulong result;
    
    __asm__ __volatile__
    (
        "lock cmpxchg    %%ebx, (%%esi)"
        : "=a" (result)
        : "a" (old_value), "b" (new_value), "S" (target)
    );
    
    return (result == old_value ? 1 : 0);
}


/*
 * Memory barriers
 */
static inline void membar()
{
    __asm__ __volatile__
    (
        "mfence"
        :
        :
    );
}

static inline void readbar()
{
    __asm__ __volatile__
    (
        "lfence"
        :
        :
    );
}

static inline void writebar()
{
    __asm__ __volatile__
    (
        "sfence"
        :
        :
    );
}


#endif
