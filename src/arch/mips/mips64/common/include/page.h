#ifndef __ARCH_MIPS64_COMMON_INCLUDE_PAGE__
#define __ARCH_MIPS64_COMMON_INCLUDE_PAGE__


#include "common/include/data.h"
#include "common/include/memory.h"


/*
 * Page
 */
struct page_table_entry {
    union {
        struct {
            u64     present         : 1;
            u64     read_allow      : 1;
            u64     write_allow     : 1;
            u64     exec_allow      : 1;
            u64     cache_allow     : 1;
            u64     has_next_level  : 1;
            u64     reserved        : 6;
            u64     pfn             : 52;
        };
        
        u64 value;
    };
} packedstruct;

struct page_frame {
    union {
        u8 bytes[PAGE_SIZE];
        struct page_table_entry entries[PAGE_SIZE / sizeof(struct page_table_entry)];
    };
} packedstruct;


#endif
