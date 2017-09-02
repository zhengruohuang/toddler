#ifndef __ARCH_MIPS32_COMMON_INCLUDE_PAGE__
#define __ARCH_MIPS32_COMMON_INCLUDE_PAGE__


#include "common/include/data.h"


/*
 * Page
 */
struct pde {
    u32     present         : 1;
    u32     write_allow     : 1;
    u32     exec_allow      : 1;
    u32     cache_allow     : 1;
    u32     reserved        : 8;
    u32     pfn             : 20;
} packedstruct;

struct pte {
    u32     present         : 1;
    u32     write_allow     : 1;
    u32     exec_allow      : 1;
    u32     cache_allow     : 1;
    u32     reserved        : 8;
    u32     pfn             : 20;
} packedstruct;

struct page_frame {
    union {
        u8 value_u8[4096];
        u32 value_u32[1024];
        
        struct pde value_pde[1024];
        struct pte value_pte[1024];
    };
} packedstruct;


#endif
