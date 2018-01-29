#ifndef __ARCH_ARMV7_COMMON_INCLUDE_MEMORY__
#define __ARCH_ARMV7_COMMON_INCLUDE_MEMORY__


#include "common/include/data.h"


#ifndef PAGE_SIZE
#define PAGE_SIZE   4096
#endif

#ifndef PAGE_BITS
#define PAGE_BITS   12
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


#ifndef PFN_TO_ADDR
#define PFN_TO_ADDR(pfn)        ((pfn) << 12)
#endif

#ifndef ADDR_TO_PFN
#define ADDR_TO_PFN(addr)       ((addr) >> 12)
#endif


#ifndef SFN_TO_ADDR
#define SFN_TO_ADDR(pfn)        ((pfn) << 20)
#endif

#ifndef ADDR_TO_SFN
#define ADDR_TO_SFN(addr)       ((addr) >> 20)
#endif



#ifndef L1TABLE_ENTRY_COUNT
#define L1TABLE_ENTRY_COUNT     4096
#endif

#ifndef L2TABLE_ENTRY_COUNT
#define L2TABLE_ENTRY_COUNT     256
#endif


#ifndef GET_L1PTE_INDEX
#define GET_L1PTE_INDEX(addr)   ((addr) >> 20)
#endif

#ifndef GET_L2PTE_INDEX
#define GET_L2PTE_INDEX(addr)   (((addr) >> 12) & 0xfful)
#endif

#ifndef GET_PAGE_OFFSET
#define GET_PAGE_OFFSET(addr)   ((addr) & 0xffful)
#endif


#endif
