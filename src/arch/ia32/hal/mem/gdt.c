#include "common/include/data.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"


static dec_per_cpu(struct gdt, system_gdt);


static void construct_entry(int entry_index, u32 base, u32 limit, u16 attri)
{
    struct gdt_descriptor *entry = &get_per_cpu(struct gdt, system_gdt)->entries[entry_index];
    
    entry->limit_low = ((u16)limit) & 0xffff;
    entry->base_low = ((u16)base) & 0xffff;
    entry->base_mid = ((u8)(base >> 16)) & 0xff;
    entry->attri1 = ((u8)attri) & 0xFF;
    entry->limit_high_attri2 = (((u8)(limit >> 16)) & 0x0F) | ((u8)(attri >> 8)) & 0xF0;
    entry->base_high = ((u8)(base >> 24)) & 0xff;
}


void init_gdt_mp()
{
    // Get current GDT
    struct gdt *cur_gdt = get_per_cpu(struct gdt, system_gdt);
    
    /*
     * Fill in the GDT
     */
    // Zero the memory
    memzero(cur_gdt, sizeof(struct gdt));
    
    // Dummy
    construct_entry(GDT_INDEX_START, 0, 0, 0);
    
    // Kernel code
    construct_entry(
        GDT_INDEX_CODE_K, 0, 0xfffff,
        GDT_DA_CR  | GDT_DA_32 | GDT_DA_LIMIT_4K | GDT_DA_DPL_KERNEL
    );
    
    // Kernel data
    construct_entry(
        GDT_INDEX_DATA_K, 0, 0xfffff,
        GDT_DA_DRW | GDT_DA_32 | GDT_DA_LIMIT_4K | GDT_DA_DPL_KERNEL
    );
    
    // User code
    construct_entry(
        GDT_INDEX_CODE_U, 0, 0xfffff,
        GDT_DA_CR  | GDT_DA_32 | GDT_DA_LIMIT_4K | GDT_DA_DPL_USER
    );
    
    // User data
    construct_entry(
        GDT_INDEX_DATA_U, 0, 0xfffff,
        GDT_DA_DRW | GDT_DA_32 | GDT_DA_LIMIT_4K | GDT_DA_DPL_USER
    );
    
    // TSS
    construct_entry(
        GDT_INDEX_TSS, *(u32*)hal_per_processor_variable_get_init(hal_tss_driver_base_per_cpu_offset), *(u32*)hal_per_processor_variable_get_init(hal_tss_driver_limit_per_cpu_offset),
        GDT_DA_TSS
    );
    
    // Per-CPU area for kernel
    construct_entry(
        GDT_INDEX_PER_CPU_K, (u32)hal_per_processor_area_get_init() + PAGE_SIZE, 1,
        GDT_DA_DRW  | GDT_DA_32 | GDT_DA_LIMIT_4K | GDT_DA_DPL_KERNEL
    );
    
    // Per-CPU area for USER
    construct_entry(
        (u32)hal_per_processor_area_get_init(), 1,
        GDT_DA_CR  | GDT_DA_32 | GDT_DA_LIMIT_4K | GDT_DA_DPL_USER
    );
    
    
    /*
     * We have finished constructing GDT, then we will construct selectors.
     * Note that selector_code_kernel should equal to value in CS, which was
     * set by OS Loader before HAL is started, so CS need not to be reloaded
     */
    cur_gdt->selectors.code_kernel = GDT_SELECTOR_CODE_K;
    cur_gdt->selectors.data_kernel = GDT_SELECTOR_DATA_K;
    
    cur_gdt->selectors.code_user = GDT_SELECTOR_CODE_U;
    cur_gdt->selectors.data_user = GDT_SELECTOR_DATA_U;
    
    cur_gdt->selectors.tss = GDT_SELECTOR_TSS;
    
    cur_gdt->selectors.per_cpu_area_kernel = GDT_SELECTOR_PER_CPU_K;
    cur_gdt->selectors.per_cpu_area_user = GDT_SELECTOR_PER_CPU_U;
    
    /*
     * Load our GDT
     */
    cur_gdt->gdtr_value.limit = sizeof(struct gdt_descriptor) * GDT_ENTRY_COUNT - 1;
    cur_gdt->gdtr_value.base = (u32)&cur_gdt->entries;
    
    __asm__ __volatile__
    (
        "lgdt   (%%ebx);"
        "jmp    RefreshGDT_MP;"         // Force to use the new GDT
        "RefreshGDT_MP:;"
        "nop;"                          // Just to have a rest
        
        // Load segment registers
        "movw   %%ax, %%ds;"
        "movw   %%ax, %%es;"
        "movw   %%ax, %%fs;"
        "movw   %%dx, %%gs;"
        "movw   %%ax, %%ss;"
        :
        : "a" (cur_gdt->selectors.data_kernel), "b" (&cur_gdt->gdtr_value), "d" (cur_gdt->selectors.per_cpu_area_kernel)
    );
}

void init_gdt()
{
    init_gdt_mp();
}
