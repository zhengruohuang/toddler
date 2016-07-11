#ifndef __ARCH_IA32_HAL_INCLUDE_MEM__
#define __ARCH_IA32_HAL_INCLUDE_MEM__


#include "common/include/data.h"


/*
 * GDT
 */
#define GDT_ENTRY_COUNT     32

#define GDT_DA_32           0x4000
#define GDT_DA_LIMIT_4K     0x8000

#define GDT_DA_DPL_KERNEL   0x00
#define GDT_DA_DPL_USER     0x60

#define GDT_DA_DR           0x90
#define GDT_DA_DRW          0x92
#define GDT_DA_DRWA         0x93
#define GDT_DA_C            0x98
#define GDT_DA_CR           0x9a
#define GDT_DA_CCO          0x9c
#define GDT_DA_CCOR         0x9e

#define GDT_DA_LDT          0x82
#define GDT_DA_TSS          0x89

#define GDT_SA_RPL_MASK     0xFFFC
#define GDT_SA_RPL_KERNEL   0
#define GDT_SA_RPL_USER     3

#define GDT_SA_TI_MASK      0xFFFB
#define GDT_SA_TIG          0
#define GDT_SA_TIL          4


/*
 * GDT Index
 */
#define GDT_INDEX_START     0

#define GDT_INDEX_CODE_K    1
#define GDT_INDEX_DATA_K    2

#define GDT_INDEX_CODE_U    3
#define GDT_INDEX_DATA_U    4

#define GDT_INDEX_TSS_USER  5
#define GDT_INDEX_TSS_IOPB  6

#define GDT_INDEX_PER_CPU_K 7
#define GDT_INDEX_PER_CPU_U 8


/*
 * GDT Selectors
 */
#define GDT_SELECTOR_CODE_K (sizeof(struct gdt_descriptor) * (GDT_INDEX_CODE_K - GDT_INDEX_START))
#define GDT_SELECTOR_DATA_K (sizeof(struct gdt_descriptor) * (GDT_INDEX_DATA_K - GDT_INDEX_START))

#define GDT_SELECTOR_CODE_U ((sizeof(struct gdt_descriptor) * (GDT_INDEX_CODE_U - GDT_INDEX_START)) | GDT_SA_RPL_USER)
#define GDT_SELECTOR_DATA_U ((sizeof(struct gdt_descriptor) * (GDT_INDEX_DATA_U - GDT_INDEX_START)) | GDT_SA_RPL_USER)

#define GDT_SELECTOR_TSS_USER   (sizeof(struct gdt_descriptor) * (GDT_INDEX_TSS_USER - GDT_INDEX_START))
#define GDT_SELECTOR_TSS_IOPB   (sizeof(struct gdt_descriptor) * (GDT_INDEX_TSS_IOPB - GDT_INDEX_START))

#define GDT_SELECTOR_PER_CPU_K  (sizeof(struct gdt_descriptor) * (GDT_INDEX_PER_CPU_K - GDT_INDEX_START))
#define GDT_SELECTOR_PER_CPU_U  ((sizeof(struct gdt_descriptor) * (GDT_INDEX_PER_CPU_U - GDT_INDEX_START)) | GDT_SA_RPL_USER)

struct gdt_selectors {
    u16     code_kernel;
    u16     data_kernel;
    
    u16     code_user;
    u16     data_user;
    
    u16     tss_user;
    u16     tss_iopb;
    
    u16     per_cpu_area_kernel;
    u16     per_cpu_area_user;
} packedstruct;

struct gdt_descriptor {
    u16     limit_low;                      // Limit
    u16     base_low;                       // Base
    u8      base_mid;                       // Base
    u8      attri1;                         // P(1) DPL(2) DT(1) TYPE(4)
    u8      limit_high_attri2;              // G(1) D(1) 0(1) AVL(1) LimitHigh(4)
    u8      base_high;                      // Base
} packedstruct;

struct gdt_register {
    u16     limit;
    u32     base;
} packedstruct;

struct gdt {
    struct gdt_register     gdtr_value;
    struct gdt_descriptor   entries[GDT_ENTRY_COUNT];
    struct gdt_selectors    selectors;
} packedstruct;

extern void init_gdt_mp();
extern void init_gdt();


/*
 * Mem mapping
 */
extern void init_user_hi4();
extern void init_user_page_dir(ulong page_dir_pfn);
extern ulong get_paddr(ulong page_dir_pfn, ulong vaddr);
extern int user_indirect_map_array(
    ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length,
    int exec, int write, int cacheable, int override
);
extern int user_indirect_unmap_array(ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length);

extern void kernel_indirect_map(ulong vaddr, ulong paddr, int disable_cache, int override);
extern void kernel_indirect_map_array(ulong vaddr, ulong paddr, size_t size, int disable_cache, int override);
extern void kernel_direct_map(ulong addr, int disable_cache);
extern void kernel_direct_map_array(ulong addr, size_t size, int disable_cache);


/*
 * TLB
 */
extern void invalidate_tlb_array(ulong vaddr, size_t size);


/*
 * HAL mem pool
 */
extern ulong palloc(int count);
extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


#endif
