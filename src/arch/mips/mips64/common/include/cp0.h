#ifndef __ARCH_MIPS64_COMMON_INCLUDE_CP0__
#define __ARCH_MIPS64_COMMON_INCLUDE_CP0__


#include "common/include/data.h"


/*
 * Generic read/write
 */
#define __dmfc0(value, reg, sel)            \
    __asm__ __volatile__ (                  \
        "dmfc0 %[r], $" #reg ", " #sel ";"  \
        : [r] "=r" (value)                  \
        :                                   \
    )

#define __dmtc0(value, reg, sel)            \
    __asm__ __volatile__ (                  \
        "dmtc0 %[r], $" #reg ", " #sel ";"  \
        :                                   \
        : [r] "r" (value)                   \
    )


/*
 * Exception
 */
struct cp0_ebase64 {
    union {
#if (ARCH_LITTLE_ENDIAN)
        struct {
            u64 cpunum      : 10;
            u64 zero0       : 1;
            u64 write_gate  : 1;
            u64 base        : 52;
        };
#else
        struct {
            u64 base        : 52;
            u64 write_gate  : 1;
            u64 zero0       : 1;
            u64 cpunum      : 10;
        };
#endif
        
        u64 value;
    };
};

#define read_cp0_ebase64(value)     __dmfc0(value, 15, 1)
#define write_cp0_ebase64(value)    __dmtc0(value, 15, 1)


/*
 * TLB
 */
struct cp0_entry_hi {
    union {
#if (ARCH_LITTLE_ENDIAN)
        struct {
            u64 asid    : 8;
            u64 asid_ex : 2;
            u64 hw_inv  : 1;
            u64 vpn2_ex : 2;
            u64 vpn2    : 49;
            u64 region  : 2;
        };
#else
        struct {
            u64 region  : 2;
            u64 vpn2    : 49;
            u64 vpn2_ex : 2;
            u64 hw_inv  : 1;
            u64 asid_ex : 2;
            u64 asid    : 8;
        };
#endif
        
        u64 value;
    };
} packedstruct;

struct cp0_page_mask64 {
    union {
#if (ARCH_LITTLE_ENDIAN)
        struct {
            u64 zero0   : 11;
            u64 mask_ex : 2;
            u64 mask    : 47;
            u64 zero1   : 4;
        };
#else
        struct {
            u64 zero1   : 4;
            u64 mask    : 47;
            u64 mask_ex : 2;
            u64 zero0   : 11;
        };
#endif
        
        u64 value;
    };
} packedstruct;

struct cp0_entry_lo {
    union {
#if (ARCH_LITTLE_ENDIAN)
        struct {
            u64 global  : 1;
            u64 valid   : 1;
            u64 dirty   : 1;
            u64 coherent: 3;
            u64 pfn     : 49;
            u64 fill    : 7;
            u64 xi      : 1;
            u64 ri      : 1;
        };
#else
        struct {
            u64 ri      : 1;
            u64 xi      : 1;
            u64 fill    : 7;
            u64 pfn     : 49;
            u64 coherent: 3;
            u64 dirty   : 1;
            u64 valid   : 1;
            u64 global  : 1;
        };
#endif
        
        u64 value;
    };
} packedstruct;

#define read_cp0_entry_hi(value)    __dmfc0(value, 10, 0)
#define write_cp0_entry_hi(value)   __dmtc0(value, 10, 0)

#define read_cp0_page_mask64(value)  __dmfc0(value, 5, 0)
#define write_cp0_page_mask64(value) __dmtc0(value, 5, 0)

#define read_cp0_entry_lo0(value)   __dmfc0(value, 2, 0)
#define write_cp0_entry_lo0(value)  __dmtc0(value, 2, 0)

#define read_cp0_entry_lo1(value)   __dmfc0(value, 3, 0)
#define write_cp0_entry_lo1(value)  __dmtc0(value, 3, 0)

#define read_cp0_bad_vaddr(value)   __dmfc0(value, 8, 0)


/*
 * Interrupt
 */
#define read_cp0_epc(value)     __dmfc0(value, 14, 0)
#define write_cp0_epc(value)    __dmtc0(value, 14, 0)


#endif
