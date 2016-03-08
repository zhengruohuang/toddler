#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/apic.h"


ulong *per_cpu_area_start_vaddr = NULL;
ulong *per_cpu_lapic_vaddr = NULL;
ulong *per_cpu_stack_top = NULL;


/*
 * This will map per-cpu area into the addr space
 */
void init_mp()
{
    kprintf("Initializing multiprocessor support\n");
    
    // Alloc mem
    per_cpu_area_start_vaddr = (ulong *)kalloc(sizeof(ulong) * num_cpus);
    per_cpu_lapic_vaddr = (ulong *)kalloc(sizeof(ulong) * num_cpus);
    per_cpu_stack_top = (ulong *)kalloc(sizeof(ulong) * num_cpus);
    
    // Reserve pages
    ulong start_pfn = palloc(PER_CPU_AREA_PPAGE_COUNT * num_cpus);
    ulong cur_top_vaddr = PER_CPU_AREA_TOP_VADDR;
    
    int i;
    for (i = 0; i < num_cpus; i++) {
        ulong cur_area_vstart = cur_top_vaddr - PER_CPU_AREA_VSIZE;
        
        // Calc addr
        ulong cur_lapic_vaddr = cur_area_vstart + PER_CPU_ARE_LAPIC_OFFSET;
        ulong cur_stack_top = cur_area_vstart + PER_CPU_ARE_STACK_TOP_OFFSET;
        
        // Map LAPIC
        kernel_indirect_map_array(cur_lapic_vaddr, lapic_paddr, PAGE_SIZE, 1);
        
        // Map per-cpu stack
        kernel_indirect_map_array(cur_area_vstart, PFN_TO_ADDR(start_pfn), PER_CPU_AREA_PSIZE, 1);
        
        // Record the result
        per_cpu_area_start_vaddr[i] = cur_area_vstart;
        per_cpu_lapic_vaddr[i] = cur_lapic_vaddr;
        per_cpu_stack_top[i] = cur_stack_top;
        
        // Tell the user
        kprintf("\tCPU #%d, LAPIC: %p -> %p (%dB), stack: %p -> %p (%dB)\n", i,
            cur_area_vstart + PER_CPU_ARE_LAPIC_OFFSET, lapic_paddr, PAGE_SIZE,
            cur_area_vstart, PFN_TO_ADDR(start_pfn), PER_CPU_AREA_PSIZE
        );
        
        // Get ready for next iteration
        cur_top_vaddr -= PER_CPU_AREA_VSIZE;
        start_pfn += PER_CPU_AREA_PPAGE_COUNT;
    }
}

/*
 * This will send IPI to start all APs
 */
void bringup_mp()
{
    
}
