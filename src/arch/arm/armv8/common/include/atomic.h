#ifndef __ARCH_ARMV7_COMMON_INCLUDE_ATOMIC__
#define __ARCH_ARMV7_COMMON_INCLUDE_ATOMIC__


#include "common/include/data.h"


/*
 * Comapre and swap
 */
static inline int atomic_cas(volatile void *target, ulong old_value, ulong new_value)
{
//     ulong failed, read;
//     
//     __asm__ __volatile__
//     (
//         "1: ldrex %[read], [%[ptr], #0];"
//         "   cmp %[read], %[val_old];"
//         "   bne 2f;"
//         "   strex %[failed], %[val_new], [%[ptr], #0];"
//         "   cmp %[failed], #1;"
//         "   beq 1b;"
//         "2:;"
//         : [failed] "=&r" (failed), [read] "=&r" (read)
//         : [ptr] "r" (target), [val_old] "r" (old_value), [val_new] "r" (new_value)
//         : "cc", "memory"
//     );
//     
//     return read == old_value;
//     
    
    
    if (*(ulong *)target == old_value) {
        *(ulong *)target = new_value;
        return 1;
    } else {
        return 0;
    }
}

static inline int atomic_cas_uint(volatile void *target, unsigned int old_value, unsigned int new_value)
{
//     unsigned int failed, read;
//     
//     __asm__ __volatile__
//     (
//         "1: ldrex %[read], [%[ptr], #0];"
//         "   cmp %[read], %[val_old];"
//         "   bne 2f;"
//         "   strex %[failed], %[val_new], [%[ptr], #0];"
//         "   cmp %[failed], #1;"
//         "   beq 1b;"
//         "2:;"
//         : [failed] "=&r" (failed), [read] "=&r" (read)
//         : [ptr] "r" (target), [val_old] "r" (old_value), [val_new] "r" (new_value)
//         : "cc", "memory"
//     );
//     
//     return read == old_value;
    
    if (*(unsigned int *)target == old_value) {
        *(unsigned int *)target = new_value;
        return 1;
    } else {
        return 0;
    }
}


/*
 * Read and write
 */
static inline void atomic_write(volatile void *target, unsigned long value)
{
//     __asm__ __volatile__(
//         "str %[val], [%[ptr], #0];"
//         :
//         : [ptr] "r" (target), [val] "r" (value)
//         : "cc", "memory"
//     );
        
    *(unsigned long *)target = value;
}


/*
 * Fetch and add
 */
static inline void atomic_inc(volatile unsigned long *target)
{
//     ulong tmp, failed;
//     
//     __asm__ __volatile__
//     (
//         "1: ldrex %[tmp], [%[ptr], #0];"
//         "   add %[tmp], %[tmp], #1;"
//         "   strex %[failed], %[tmp], [%[ptr], #0];"
//         "   cmp %[failed], #1;"
//         "   beq 1b;"
//         : [tmp] "=&r" (tmp), [failed] "=&r" (failed)
//         : [ptr] "r" (target)
//         : "cc", "memory"
//     );
    
    *target = *target + 1;
}


/*
 * Memory barriers
 */
static inline void atomic_membar()
{
//     __asm__ __volatile__
//     (
//         "dmb;"
//         :
//         :
//         : "memory"
//     );
}

static inline void atomic_readbar()
{
    atomic_membar();
}

static inline void atomic_writebar()
{
    atomic_membar();
}


#endif
