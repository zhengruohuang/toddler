#ifndef __ARCH_ARMV7_COMMON_INCLUDE_MEMORY__
#define __ARCH_ARMV7_COMMON_INCLUDE_MEMORY__


#include "common/include/data.h"


#ifndef PAGE_SIZE
#define PAGE_SIZE   4096
#endif

#ifndef PAGE_BITS
#define PAGE_BITS   12
#endif

#ifndef PAGE_ENTRY_COUNT
#define PAGE_ENTRY_COUNT    1024
#endif

#ifndef PAGE_LEVEL
#define PAGE_LEVEL  2
#endif

#ifndef ALIGN_MIN
#define ALIGN_MIN   1
#endif

#ifndef ALIGN_MAX
#define ALIGN_MAX   64
#endif

#ifndef ALIGN_DEFAULT
#define ALIGN_DEFAULT   4
#endif

#ifndef PFN_TO_ADDR
#define PFN_TO_ADDR(pfn)        ((pfn) << 12)
#endif

#ifndef ADDR_TO_PFN
#define ADDR_TO_PFN(addr)       ((addr) >> 12)
#endif

#ifndef GET_L1PTE_INDEX
#define GET_L1PTE_INDEX(addr)   ((addr) >> 22)
#endif

#ifndef GET_L2PTE_INDEX
#define GET_L2PTE_INDEX(addr)   (((addr) << 10) >> 22)
#endif

#ifndef GET_PAGE_OFFSET
#define GET_PAGE_OFFSET(addr)   (((addr) << 20) >> 20)
#endif


/* Architecture Data Types */
#define PTE_CACHE_WRITE_BACK_ALLOCATE   0x1

struct l1pte {
    u32     present         : 1;
    u32     reserved1       : 2;
    u32     non_secure      : 1;
    u32     reserved2       : 1;
    u32     domain          : 4;
    u32     reserved3       : 1;
    u32     pfn             : 20;
} packedstruct;

struct l2pte {
    u32     non_exec        : 1;
    u32     present         : 1;
    u32     cache_inner     : 2;
    u32     supervisor      : 1;
    u32     accessible      : 1;
    u32     cache_outer     : 2;
    u32     cacheable       : 1;
    u32     read_only       : 1;
    u32     shareable       : 1;
    u32     non_global      : 1;
    u32     pfn             : 20;
} packedstruct;

struct page_frame {
    union {
        u8 value_u8[4096];
        u32 value_u32[1024];
        
        struct l1pte value_l1pte[1024];
        struct l2pte value_l2pte[1024];
    };
} packedstruct;


#endif
