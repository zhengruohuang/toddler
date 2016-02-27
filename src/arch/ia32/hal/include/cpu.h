#ifndef __ARCH_IA32_HAL_INCLUDE_CPU__
#define __ARCH_IA32_HAL_INCLUDE_CPU__


#include "common/include/data.h"


#define dec_per_cpu(type, name)         \
    int __##name##_per_cpu_id = -1
    
#define ext_per_cpu(type, name)         \
    extern int __##name##_per_cpu_id
    
#define get_per_cpu(type, name)   ((type *)access_per_cpu_var(&__##name##_per_cpu_index, sizeof(type)))


struct cpuid_reg {
    ulong     a;
    ulong     b;
    ulong     c;
    ulong     d;
};


extern void init_cpuid();
extern int cpuid(struct cpuid_reg *reg);

extern int get_num_cpus();
extern void init_topo();


#endif
