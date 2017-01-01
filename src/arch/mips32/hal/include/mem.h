#ifndef __ARCH_MIPS32_HAL_INCLUDE_MEM__
#define __ARCH_MIPS32_HAL_INCLUDE_MEM__


#include "common/include/data.h"
#include "hal/include/cpu.h"


/*
 * Page map
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


/*
 * TLB
 */
struct tlb_entry_hi {
    union {
        struct {
            u32 asid : 8;
            u32 zero : 5;
            u32 vpn2 : 19;
        };
        
        u32 value;
    };
} packedstruct;

struct tlb_page_mask {
    union {
        struct {
            u32 zero0 : 13;
            u32 mask : 16;
            u32 zero1 : 3;
        };
        
        u32 value;
    };
} packedstruct;

struct tlb_entry_lo {
    union {
        struct {
            u32 global : 1;
            u32 valid : 1;
            u32 dirty : 1;
            u32 coherent : 3;
            u32 pfn : 26;
        };
        
        u32 value;
    };
} packedstruct;

struct tlb_entry {
    struct tlb_entry_hi hi;
    struct tlb_page_mask pm;
    
    struct tlb_entry_lo lo0;
    struct tlb_entry_lo lo1;
} packedstruct;

ext_per_cpu(struct page_frame *, cur_page_dir);

extern int reserve_tlb_entry();
extern void write_tlb_entry(int index, u32 hi, u32 pm, u32 lo0, u32 lo1);

extern void tlb_refill_kernel(u32 addr);
extern void tlb_refill_user(u32 addr);

extern void init_tlb();


/*
 * KAlloc
 */
extern ulong palloc(int count);
extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


#endif
