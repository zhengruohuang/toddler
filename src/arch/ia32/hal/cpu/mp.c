#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/apic.h"


ulong get_per_cpu_area_start_vaddr(int cpu_id)
{
    assert(cpu_id < num_cpus);
    
    return PER_CPU_AREA_TOP_VADDR - PER_CPU_AREA_SIZE * (cpu_id + 1);
}

ulong get_per_cpu_stack_top(int cpu_id)
{
    return get_per_cpu_area_start_vaddr(cpu_id) + PER_CPU_AREA_STACK_TOP_OFFSET;
}

ulong get_per_cpu_data_vaddr(int cpu_id)
{
    return get_per_cpu_area_start_vaddr(cpu_id) + PER_CPU_AREA_DATA_START_OFFSET;
}

/*
 * This will map per-cpu area into the addr space
 */
void init_mp()
{
    kprintf("Initializing multiprocessor support\n");
    
    // Reserve pages
    ulong start_pfn = palloc(PER_CPU_AREA_PAGE_COUNT * num_cpus);
    ulong cur_top_vaddr = PER_CPU_AREA_TOP_VADDR;
    
    int i;
    for (i = 0; i < num_cpus; i++) {
        ulong cur_area_vstart = cur_top_vaddr - PER_CPU_AREA_SIZE;

        // Map per-cpu stack
        kernel_indirect_map_array(cur_area_vstart, PFN_TO_ADDR(start_pfn), PER_CPU_AREA_SIZE, 1, 0);
        
        // Tell the user
        kprintf("\tCPU #%d, stack: %p -> %p (%dB)\n", i,
            cur_area_vstart, PFN_TO_ADDR(start_pfn), PER_CPU_AREA_SIZE
        );
        
        // Get ready for next iteration
        cur_top_vaddr -= PER_CPU_AREA_SIZE;
        start_pfn += PER_CPU_AREA_PAGE_COUNT;
    }
}

/*
 * This will send IPI to start all APs
 */
void bringup_mp()
{
    
}
