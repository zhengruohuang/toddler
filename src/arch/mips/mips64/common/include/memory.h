#ifndef __ARCH_MIPS64_COMMON_INCLUDE_MEMORY__
#define __ARCH_MIPS64_COMMON_INCLUDE_MEMORY__


#include "common/include/memlayout.h"


#ifndef PAGE_SIZE
#define PAGE_SIZE           4096
#endif

#ifndef PAGE_BITS
#define PAGE_BITS           12
#endif

#ifndef PAGE_ENTRY_COUNT
#define PAGE_ENTRY_COUNT    512
#endif

#ifndef PAGE_ENTRY_BITS
#define PAGE_ENTRY_BITS     9
#endif

#ifndef PAGE_LEVELS
#define PAGE_LEVELS         4
#endif

#ifndef VADDR_BITS
#define VADDR_BITS          ((PAGE_ENTRY_BITS * PAGE_LEVELS + PAGE_BITS))
#endif


#ifndef ALIGN_MIN
#define ALIGN_MIN           8
#endif

#ifndef ALIGN_MAX
#define ALIGN_MAX           64
#endif

#ifndef ALIGN_DEFAULT
#define ALIGN_DEFAULT       8
#endif


/*
 * MIPS page mask
 */
#define KERNEL_PAGE_MASK        0x3ffff
#define KERNEL_PAGE_MASK_EX     0x3

#define USER_PAGE_MASK          0
#define USER_PAGE_MASK_EX       0x3

#define TLB_PFN_SHIFT_BITS      12
#define TLB_VPN2_SHIFT_BITS     13


/*
 * Addr <--> PFN
 */
#ifndef PFN_TO_ADDR
#define PFN_TO_ADDR(pfn)        ((pfn) << PAGE_BITS)
#endif

#ifndef ADDR_TO_PFN
#define ADDR_TO_PFN(addr)       ((addr) >> PAGE_BITS)
#endif


/*
 * Physical addr <--> HAL/Kernel code addr (KCODE)
 */
#ifndef PHYS_TO_KCODE
#define PHYS_TO_KCODE(addr)     ((addr) | SEG_LOW_CACHED)
#endif

#ifndef KCODE_TO_PHYS
#define KCODE_TO_PHYS(addr)     ((addr) & ~SEG_LOW_CACHED)
#endif


/*
 * Physical addr <--> HAL/Kernel data addr (KDATA)
 */
#ifndef PHYS_TO_KDATA
#define PHYS_TO_KDATA(addr)     ((addr) | SEG_CACHED)
#endif

#ifndef KDATA_TO_PHYS
#define KDATA_TO_PHYS(addr)     ((addr) & ~SEG_CACHED)
#endif


#endif
