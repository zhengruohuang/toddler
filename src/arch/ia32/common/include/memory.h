#ifndef __ARCH_IA32_COMMON_INCLUDE_MEMORY__
#define __ARCH_IA32_COMMON_INCLUDE_MEMORY__


#include "common/include/data.h"


#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef PAGE_ENTRY_COUNT
#define PAGE_ENTRY_COUNT 1024
#endif

#ifndef PAGE_LEVEL
#define PAGE_LEVEL 2
#endif

#ifndef ALIGN_MIN
#define ALIGN_MIN 1
#endif

#ifndef ALIGN_MAX
#define ALIGN_MAX 2048
#endif

#ifndef ALIGN_DEFAULT
#define ALIGN_DEFAULT 4
#endif

#ifndef HAL_MEMORY_GET_PHYSICAL_ADDRESS_BY_PFN
#define HAL_MEMORY_GET_PHYSICAL_ADDRESS_BY_PFN(pfn)     ((pfn) << 12)
#endif

#ifndef HAL_MEMORY_GET_PFN_BY_PHYSICAL_ADDRESS
#define HAL_MEMORY_GET_PFN_BY_PHYSICAL_ADDRESS(addr)    ((addr) >> 12)
#endif

#ifndef HAL_MEMORY_GET_PDE_INDEX
#define HAL_MEMORY_GET_PDE_INDEX(addr)          ((addr) >> 22)
#endif

#ifndef HAL_MEMORY_GET_PTE_INDEX
#define HAL_MEMORY_GET_PTE_INDEX(addr)          (((addr) << 10) >> 22)
#endif

#ifndef HAL_MEMORY_GET_INDEX_IN_PAGE
#define HAL_MEMORY_GET_INDEX_IN_PAGE(addr)      (((addr) << 20) >> 20)
#endif

#ifndef HAL_VADDR_TO_PADDR
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

struct gdt_descriptor {
    u16     limit_low;          // Limit
    u16     base_low;           // Base
    u8      base_mid;           // Base
    u8      attri1;             // P(1) DPL(2) DT(1) TYPE(4)
    u8      limit_high_attri2;  // G(1) D(1) 0(1) AVL(1) LimitHigh(4)
    u8      base_high;          // Base
} packedstruct;

struct tss {
    u32     backlink;
    u32     esp0;   // stack pointer to use during interrupt
    u32     ss0;    // stack segment to use during interrupt
    u32     esp1;
    u32     ss1;
    u32     esp2;
    u32     ss2;
    u32     cr3;
    u32     eip;
    u32     flags;
    u32     eax;
    u32     ecx;
    u32     edx;
    u32     ebx;
    u32     esp;
    u32     ebp;
    u32     esi;
    u32     edi;
    u32     es;
    u32     cs;
    u32     ss;
    u32     ds;
    u32     fs;
    u32     gs;
    u32     ldt;
    u16     trap;
    u16     iopb_addr;     
    /* Note that if the base address of IOPB is
     * greater than or equal to the limit(size) of
     * the TSS segment (normally it would be 104),
     * it means that there's no IOPB */
} packedstruct;   


#endif
