#ifndef __ARCH_SPARC_HAL_INCLUDE_CPU__
#define __ARCH_SPARC_HAL_INCLUDE_CPU__


#include "common/include/data.h"


/*
 * CPU ID
 */
struct cpu_prop {
    int dcache_size, icache_size;
    int dcache_sets, icache_sets;
    int dcache_block_size, icache_block_size;
    
    int tlb_sets, tlb_size;
    
    ulong freq_base, freq_clock, freq_bus;
};

struct pvr_record {
    ulong pvr;
    char *class;
    char *model;
    char *rev;
};

extern struct cpu_prop cpu_info;

extern void init_cpuid();


/*
 * Topo
 */
extern int num_cpus;
extern void init_topo();


/*
 * MP
 */
extern int get_cpu_id();

extern ulong get_per_cpu_area_start_paddr(int cpu_id);
extern ulong get_my_cpu_area_start_paddr();

extern ulong get_per_cpu_area_start_vaddr(int cpu_id);
extern ulong get_my_cpu_area_start_vaddr();

extern void init_mp();


#endif
