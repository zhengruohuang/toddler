#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "common/include/proc.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"


static ulong per_cpu_area_start_vaddr = 0;

ulong tcb_padded_size = 0;
ulong tcb_area_start_vaddr = 0;
ulong tcb_area_size = 0;


/*
 * Get my CPU id
 */
int get_cpu_id()
{
    u32 cpu_num = 0;
    
    return (int)cpu_num;
}


/*
 * Per-CPU private data
 */
ulong get_per_cpu_area_start_vaddr(int cpu_id)
{
    assert(cpu_id < num_cpus);
    return per_cpu_area_start_vaddr + PER_CPU_AREA_SIZE * cpu_id;
}

ulong get_my_cpu_area_start_vaddr()
{
    return get_per_cpu_area_start_vaddr(get_cpu_id());
}


/*
 * Init
 */
void init_mp()
{
    int i;
    
    kprintf("Initializing multiprocessor support\n");
    
    // Reserve pages
    ulong per_cpu_are_start_pfn = palloc(PER_CPU_AREA_PAGE_COUNT * num_cpus);
    per_cpu_area_start_vaddr = PER_CPU_AREA_BASE_VADDR;
    
    // Map per CPU private area
    ulong cur_area_pstart = PFN_TO_ADDR(per_cpu_are_start_pfn);
    ulong cur_area_vstart = per_cpu_area_start_vaddr;
    
    kprintf("\tPer-CPU area start phys @ %p, virt @ %p\n", (void *)cur_area_pstart, (void *)cur_area_vstart);
    
    for (i = 0; i < num_cpus; i++) {
        // Map the page to its new virt location
        kernel_map_per_cpu_area(cur_area_vstart, cur_area_pstart);
        fill_kernel_pht(cur_area_vstart, PER_CPU_AREA_SIZE, 0, 1);
        evict_kernel_pht(cur_area_pstart, PER_CPU_AREA_SIZE);
        
        // Tell the user
        kprintf("\tCPU #%d, per-CPU area: %p (%d Bytes)\n", i,
            (void *)cur_area_vstart, PER_CPU_AREA_SIZE
        );
        
        cur_area_pstart += PER_CPU_AREA_SIZE;
        cur_area_vstart += PER_CPU_AREA_SIZE;
    }
}
