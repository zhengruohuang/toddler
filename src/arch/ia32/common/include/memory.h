#ifndef __ARCH_IA32_COMMON_INCLUDE_MEMORY__
#define __ARCH_IA32_COMMON_INCLUDE_MEMORY__


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
#define ALIGN_MAX   2048
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

#ifndef GET_PDE_INDEX
#define GET_PDE_INDEX(addr)     ((addr) >> 22)
#endif

#ifndef GET_PTE_INDEX
#define GET_PTE_INDEX(addr)     (((addr) << 10) >> 22)
#endif

#ifndef GET_PAGE_OFFSET
#define GET_PAGE_OFFSET(addr)   (((addr) << 20) >> 20)
#endif

#ifndef HAL_VADDR_TO_PADDR
// FIXME: This is incorrect
#define HAL_VADDR_TO_PADDR(vaddr)               (vaddr - (HAL_VIRTUAL_MEMORY_START_ADDRESS - HAL_EXECUTE_START_PFN * PAGE_SIZE))
#endif


/* Architecture Data Types */
#define HAL_PAGE_PG_P0  0       /* P       The page not exists          */
#define HAL_PAGE_PG_P   1       /* P       The page exists              */
#define HAL_PAGE_PG_RWR 0       /* R/W     Read & Execute               */
#define HAL_PAGE_PG_RWW 1       /* R/W     Read & WRite & Execute       */
#define HAL_PAGE_PG_USS 0       /* U/S     Supervisor                   */
#define HAL_PAGE_PG_USU 1       /* U/S     User                         */

struct pde {
    u32     present         : 1;
    u32     rw              : 1;
    u32     user            : 1;
    u32     write_through   : 1;
    u32     cache_disabled  : 1;
    u32     accessed        : 1;
    u32     reserved        : 1;
    u32     page_size       : 1;
    u32     global          : 1;
    u32     available       : 3;
    u32     pfn             : 20;
} packedstruct;

struct pte {
    u32     present         : 1;
    u32     rw              : 1;
    u32     user            : 1;
    u32     write_through   : 1;
    u32     cache_disabled  : 1;
    u32     accessed        : 1;
    u32     dirty           : 1;
    u32     page_attri_table: 1;
    u32     global          : 1;
    u32     available       : 3;
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
