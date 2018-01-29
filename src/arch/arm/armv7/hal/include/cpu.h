#ifndef __ARCH_ARMV7_HAL_INCLUDE_CPU__
#define __ARCH_ARMV7_HAL_INCLUDE_CPU__


#include "common/include/data.h"


/*
 * CPU ID
 */
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
