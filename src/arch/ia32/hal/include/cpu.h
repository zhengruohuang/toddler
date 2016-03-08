#ifndef __ARCH_IA32_HAL_INCLUDE_CPU__
#define __ARCH_IA32_HAL_INCLUDE_CPU__


#include "common/include/data.h"


/*
 * CPUID
 */
struct cpuid_reg {
    ulong a;
    ulong b;
    ulong c;
    ulong d;
};

extern void init_cpuid();
extern int cpuid(struct cpuid_reg *reg);


/*
 * Per-CPU var
 */
#ifndef dec_per_cpu
#define dec_per_cpu(type, name) int __##name##_per_cpu_id = -1
#endif

#ifndef ext_per_cpu
#define ext_per_cpu(type, name) extern int __##name##_per_cpu_id
#endif

#ifndef get_per_cpu
#define get_per_cpu(type, name)   ((type *)access_per_cpu_var(&__##name##_per_cpu_index, sizeof(type)))
#endif

extern void *access_per_cpu_var(int *id, size_t size);


/*
 * Topology
 */
extern int num_cpus;

extern void init_topo();


/*
 * MP
 */
extern void init_mp();
extern void bringup_mp();


#endif
