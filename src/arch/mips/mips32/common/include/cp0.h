#ifndef __ARCH_MIPS32_COMMON_INCLUDE_CP0__
#define __ARCH_MIPS32_COMMON_INCLUDE_CP0__


#include "common/include/data.h"


/*
 * TLB
 */
struct cp0_entry_hi {
    union {
#if (ARCH_LITTLE_ENDIAN)
        struct {
            u32 asid    : 8;
            u32 asid_ex : 2;
            u32 hw_inv  : 1;
            u32 vpn2_ex : 2;
            u32 vpn2    : 19;
        };
#else
        struct {
            u32 vpn2    : 19;
            u32 vpn2_ex : 2;
            u32 hw_inv  : 1;
            u32 asid_ex : 2;
            u32 asid    : 8;
        };
#endif
        
        u32 value;
    };
} packedstruct;

struct cp0_entry_lo {
    union {
#if (ARCH_LITTLE_ENDIAN)
        struct {
            u32 global  : 1;
            u32 valid   : 1;
            u32 dirty   : 1;
            u32 coherent : 3;
            u32 pfn     : 24;
            u32 no_exec : 1;
            u32 no_read : 1;
        };
#else
        struct {
            u32 no_read : 1;
            u32 no_exec : 1;
            u32 pfn     : 24;
            u32 coherent : 3;
            u32 dirty   : 1;
            u32 valid   : 1;
            u32 global  : 1;
        };
#endif
        
        u32 value;
    };
} packedstruct;

#define read_cp0_entry_hi(value)    __mfc0(value, 10, 0)
#define write_cp0_entry_hi(value)   __mtc0(value, 10, 0)

#define read_cp0_entry_lo0(value)   __mfc0(value, 2, 0)
#define write_cp0_entry_lo0(value)  __mtc0(value, 2, 0)

#define read_cp0_entry_lo1(value)   __mfc0(value, 3, 0)
#define write_cp0_entry_lo1(value)  __mtc0(value, 3, 0)

#define read_cp0_bad_vaddr(value)   __mfc0(value, 8, 0)


/*
 * Interrupt
 */
#define read_cp0_epc(value)     __mfc0(value, 14, 0)
#define write_cp0_epc(value)    __mtc0(value, 14, 0)


#endif
