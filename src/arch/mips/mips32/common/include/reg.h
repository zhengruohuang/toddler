#ifndef __ARCH_MIPS32_COMMON_INCLUDE_REG__
#define __ARCH_MIPS32_COMMON_INCLUDE_REG__


#include "common/include/data.h"


/*
 * Status
 */
struct cp0_status {
    union {
        struct {
            u32 ie      : 1;
            u32 exl     : 1;
            u32 erl     : 1;
            u32 ksu     : 2;
            u32 ux      : 1;
            u32 sx      : 1;
            u32 kx      : 1;
            u32 im      : 8;
            u32 mid     : 2;
            u32 zero1   : 1;
            u32 nmi     : 1;
            u32 sr      : 1;
            u32 zero2   : 1;
            u32 bev     : 1;
            u32 px      : 1;
            u32 mx      : 1;
            u32 zero3   : 1;
            u32 fr      : 1;
            u32 rp      : 1;
            u32 cu0     : 1;
            u32 cu1     : 1;
            u32 cu2     : 1;
            u32 zero4   : 1;
        };
        
        u32 value;
    };
};


/*
 * TLB
 */
struct tlb_entry_hi {
    union {
#if ARCH_LITTLE_ENDIAN
        struct {
            u32 asid    : 8;
            u32 zero    : 5;
            u32 vpn2    : 19;
        };
#else
        struct {
            u32 vpn2    : 19;
            u32 zero    : 5;
            u32 asid    : 8;
        };
#endif
        
        u32 value;
    };
} packedstruct;

struct tlb_page_mask {
    union {
#if ARCH_LITTLE_ENDIAN
        struct {
            u32 zero0   : 13;
            u32 mask    : 16;
            u32 zero1   : 3;
        };
#else
        struct {
            u32 zero1   : 3;
            u32 mask    : 16;
            u32 zero0   : 13;
        };
#endif
        
        u32 value;
    };
} packedstruct;

struct tlb_entry_lo {
    union {
#if ARCH_LITTLE_ENDIAN
        struct {
            u32 global  : 1;
            u32 valid   : 1;
            u32 dirty   : 1;
            u32 coherent : 3;
            u32 pfn     : 26;
        };
#else
        struct {
            u32 pfn     : 26;
            u32 coherent : 3;
            u32 dirty   : 1;
            u32 valid   : 1;
            u32 global  : 1;
        };
#endif
        
        u32 value;
    };
} packedstruct;



#endif
