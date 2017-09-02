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
    
    __asm__ __volatile__ (
        "rdhwr  %0, $0;"
        :
        : "r" (cpu_num)
    );
    
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
    kprintf("Initializing multiprocessor support\n");
    
    // Reserve pages
    ulong per_cpu_are_start_pfn = palloc(PER_CPU_AREA_PAGE_COUNT * num_cpus);
    per_cpu_area_start_vaddr = PHYS_TO_KCODE(PFN_TO_ADDR(per_cpu_are_start_pfn));
    kprintf("\tPer-CPU area start vaddr @ %lx\n", per_cpu_area_start_vaddr);
    
    // Map per CPU private area
    int i;
    for (i = 0; i < num_cpus; i++) {
        ulong cur_are_pfn = per_cpu_are_start_pfn + i * PER_CPU_AREA_PAGE_COUNT;
        ulong cur_area_vstart = PHYS_TO_KCODE(PFN_TO_ADDR(cur_are_pfn));
        
        // Tell the user
        kprintf("\tCPU #%d, per-CPU area @ %p (%dB)\n", i,
            (void *)cur_area_vstart, PER_CPU_AREA_SIZE
        );
    }
}
