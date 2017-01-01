#ifndef __ARCH_MIPS32_HAL_INCLUDE_CPU__
#define __ARCH_MIPS32_HAL_INCLUDE_CPU__


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
 * Per-CPU var
 */
#ifndef dec_per_cpu
#define dec_per_cpu(type, name) int __##name##_per_cpu_offset = -1
#endif

#ifndef ext_per_cpu
#define ext_per_cpu(type, name) extern int __##name##_per_cpu_offset
#endif

#ifndef get_per_cpu
#define get_per_cpu(type, name)   ((type *)access_per_cpu_var(&__##name##_per_cpu_offset, sizeof(type)))
#endif

extern void *access_per_cpu_var(int *id, size_t size);


/*
 * MP
 */
extern int get_cpu_id();

extern ulong get_per_cpu_area_start_vaddr(int cpu_id);
extern ulong get_my_cpu_area_start_vaddr();

extern ulong get_per_cpu_tcb_start_vaddr(int cpu_id);
extern ulong get_my_cpu_tcb_start_vaddr();

extern void init_mp();


#endif
