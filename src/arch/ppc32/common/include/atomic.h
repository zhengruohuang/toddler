#ifndef __ARCH_PPC32_COMMON_INCLUDE_ATOMIC__
#define __ARCH_PPC32_COMMON_INCLUDE_ATOMIC__


#include "common/include/data.h"


/*
 * Comapre and swap
 */
static inline int atomic_cas(volatile void *target, ulong old_value, ulong new_value)
{
    /*
     * loop:   lwarx   r6,0,r3          # Load and reserve
        cmpw    r4,r6            # Are the first two operands
                                 # equal?
        bne-    exit             # Skip if not equal
        stwcx.  r5,0,r3          # Store new value if still
                                 # reserved
        bne-    loop             # Loop if lost reservation
exit:   mr      r4,r6            # Return value from storage

     */
    register unsigned long tmp;
    
    __asm__ __volatile__
    (
        "1:;"
        "lwarx %[tmp], 0, %[ptr];"
        "cmpw %[val_old], %[tmp];"
        "bne- 2f;"
        "stwcx. %[val_new], 0, %[ptr];"
        "bne- 1b;"
        "2:;"
        : [tmp]"=&r"(tmp)
        : [ptr]"r"(target), [val_old]"r"(old_value), [val_new]"r"(new_value)
    );
    
    return tmp == old_value;
    
//     int result = 0;
//     
//     if (*(ulong *)target == old_value) {
//         *(ulong *)target = new_value;
//         result = 1;
//     }
//     
//     return result;
}

static inline int atomic_cas_uint(volatile void *target, unsigned int old_value, unsigned int new_value)
{
    register unsigned int tmp;
    
    __asm__ __volatile__
    (
        "1:;"
        "lwarx %[tmp], 0, %[ptr];"
        "cmpw %[val_old], %[tmp];"
        "bne- 2f;"
        "stwcx. %[val_new], 0, %[ptr];"
        "bne- 1b;"
        "2:;"
        : [tmp]"=&r"(tmp)
        : [ptr]"r"(target), [val_old]"r"(old_value), [val_new]"r"(new_value)
    );
    
    return tmp == old_value;

//     int result = 0;
//     
//     if (*(unsigned int *)target == old_value) {
//         *(unsigned int *)target = new_value;
//         result = 1;
//     }
//     
//     return result;
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
    register unsigned long tmp;
    
    __asm__ __volatile__ (
        "1:;"
        "lwarx %[tmp], 0, %[ptr];"
        "addic %[tmp], %[tmp], 1;"
        "stwcx. %[tmp], 0, %[ptr];"
        "bne- 1b;"
        : [tmp]"=&r"(tmp)
        : [ptr]"r"(target)
        : "cc"
    );
}


/*
 * Memory barriers
 */
static inline void atomic_membar()
{
    __asm__ __volatile__ ( "sync;" : : );
}

static inline void atomic_readbar()
{
    __asm__ __volatile__ ( "sync;" : : );
}

static inline void atomic_writebar()
{
    __asm__ __volatile__ ( "sync;" : : );
}


#endif
