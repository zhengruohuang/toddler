#ifndef __ARCH_PPC32_COMMON_INCLUDE_MEMORY__
#define __ARCH_PPC32_COMMON_INCLUDE_MEMORY__



#ifndef PAGE_SIZE
#define PAGE_SIZE   4096
#endif

#ifndef PAGE_BITS
#define PAGE_BITS   12
#endif

#ifndef PAGE_ENTRY_BITS
#define PAGE_ENTRY_BITS     10
#endif

#ifndef PAGE_ENTRY_COUNT
#define PAGE_ENTRY_COUNT    1024
#endif

#ifndef PAGE_LEVELS
#define PAGE_LEVELS 2
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


/*
 * Addr <--> PFN
 */
#ifndef PFN_TO_ADDR
#define PFN_TO_ADDR(pfn)        ((pfn) << 12)
#endif

#ifndef ADDR_TO_PFN
#define ADDR_TO_PFN(addr)       ((addr) >> 12)
#endif


/*
 * Page table entry
 */
#ifndef GET_PDE_INDEX
#define GET_PDE_INDEX(addr)     ((addr) >> 22)
#endif

#ifndef GET_PTE_INDEX
#define GET_PTE_INDEX(addr)     (((addr) << 10) >> 22)
#endif

#ifndef GET_PAGE_OFFSET
#define GET_PAGE_OFFSET(addr)   (((addr) << 20) >> 20)
#endif


/*
 * Page frame
 */
struct pde {
    u32     present         : 1;
    u32     supervisor      : 1;
    u32     write_allow     : 1;
    u32     exec_allow      : 1;
    u32     cache_allow     : 1;
    u32     next_level      : 1;
    u32     reserved        : 6;
    u32     pfn             : 20;
} packedstruct;

struct pte {
    u32     present         : 1;
    u32     supervisor      : 1;
    u32     write_allow     : 1;
    u32     exec_allow      : 1;
    u32     cache_allow     : 1;
    u32     reserved        : 7;
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


/*
 * Page hash table
 */
struct pht_entry {
    union {
        u32 word0;
        struct {
            u32     valid       : 1;
            u32     vsid        : 24;
            u32     secondary   : 1;
            u32     page_idx    : 6;
        };
    };
    
    union {
        u32 word1;
        
        struct {
            u32     pfn         : 20;
            u32     reserved1   : 3;
            u32     reference   : 1;
            u32     change      : 1;
            u32     write_thru  : 1;
            u32     no_cache    : 1;
            u32     coherent    : 1;
            u32     guarded     : 1;
            u32     reserved2   : 1;
            u32     protect     : 2;
        };
    };
} packedstruct;

struct pht_group {
    struct pht_entry entries[8];
} packedstruct;


/*
 * OS PHT attribute table
 */
struct pht_attri_entry {
    union {
        u8 value;
        struct {
            u8 persist      : 1;
            u8 temporary    : 1;
            u8 reserved     : 6;
        };
    };
} packedstruct;

struct pht_attri_group {
    struct pht_attri_entry entries[8];
} packedstruct;


/*
 * Segment register
 */
struct seg_reg {
    union {
        struct {
            u32     direct_store    : 1;
            u32     key_kernel      : 1;
            u32     key_user        : 1;
            u32     no_exec         : 1;
            u32     reserved        : 4;
            u32     vsid            : 24;
        };
        
        u32 value;
    };
} packedstruct;


#endif
