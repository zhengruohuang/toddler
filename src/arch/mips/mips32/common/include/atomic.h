#ifndef __ARCH_MIPS32_COMMON_INCLUDE_ATOMIC__
#define __ARCH_MIPS32_COMMON_INCLUDE_ATOMIC__


#include "common/include/data.h"


/*
 * Comapre and swap
 */
static inline int atomic_cas(volatile void *target, ulong old_value, ulong new_value)
{
    register ulong tmp, read;
    
    __asm__ __volatile__
    (
        "1:;"
        "   ll %[read], 0(%[ptr]);"
        "   bne %[read], %[val_old], 2f;"
        "   nop;"
        "   move %[tmp], %[val_new];"
        "   sc %[tmp], 0(%[ptr]);"
        "   beq %[tmp], %[zero], 1b;"
        "   nop;"
        "2:;"
        : [tmp] "=&r" (tmp), [read] "=&r" (read)
        : [ptr] "r" (target), [zero] "i" (0),
            [val_old] "r" (old_value), [val_new] "r" (new_value)
    );
    
    return read == old_value;
}

static inline int atomic_cas_uint(volatile void *target, unsigned int old_value, unsigned int new_value)
{
    register unsigned int tmp, read;
    
    __asm__ __volatile__
    (
        "1:;"
        "   ll %[read], 0(%[ptr]);"
        "   bne %[read], %[val_old], 2f;"
        "   nop;"
        "   move %[tmp], %[val_new];"
        "   sc %[tmp], 0(%[ptr]);"
        "   beq %[tmp], %[zero], 1b;"
        "   nop;"
        "2:;"
        : [tmp] "=&r" (tmp), [read] "=&r" (read)
        : [ptr] "r" (target), [zero] "i" (0),
            [val_old] "r" (old_value), [val_new] "r" (new_value)
    );
    
    return read == old_value;
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
    register ulong tmp;
    
    __asm__ __volatile__
    (
        "1:;"
        "   ll %[tmp], 0(%[ptr]);"
        "   addiu %[tmp], %[tmp], 0x1;"
        "   sc %[tmp], 0(%[ptr]);"
        "   beq %[tmp], %[zero], 1b;"
        "   nop;"
        : [tmp] "=&r" (tmp)
        : [ptr] "r" (target), [zero] "i" (0)
    );
}


/*
 * Memory barriers
 */
static inline void atomic_membar()
{
    __asm__ __volatile__
    (
        "sync;"
        :
        :
    );
}

static inline void atomic_readbar()
{
    __asm__ __volatile__
    (
        "sync;"
        :
        :
    );
}

static inline void atomic_writebar()
{
    __asm__ __volatile__
    (
        "sync;"
        :
        :
    );
}


#endif
