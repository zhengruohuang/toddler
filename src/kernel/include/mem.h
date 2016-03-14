#ifndef __KERNEL_INCLUDE_MEM__
#define __KERNEL_INCLUDE_MEM__


#include "common/include/data.h"


/*
 * PFN database
 */
struct pfndb_entry {
    union {
        u16 flags;
        
        struct {
            u16 usable      : 1;
            u16 mapped      : 1;
            u16 tag         : 4;
            u16 inuse       : 1;
            u16 zeroed      : 1;
            u16 kernel      : 1;
            u16 swappable   : 1;
        };
    };
} packedstruct;

extern struct pfndb_entry *get_pfn_entry_by_pfn(ulong pfn);
extern struct pfndb_entry *get_pfn_entry_by_paddr(ulong paddr);
extern void reserve_pfndb_mem(ulong start, ulong size);
extern void init_pfndb();


/*
 * Page frame allocator
 */
extern void init_palloc();
extern void palloc_test();


#endif
