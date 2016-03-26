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
 * MSR
 */
extern void msr_read(ulong reg, u64* value_out);
extern void msr_write(ulong reg, u64* value);
extern void msr_timestamp(u64* value);


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
 * Topology
 */
extern int num_cpus;

extern void init_topo();


/*
 * MP
 */
extern ulong tcb_padded_size;
extern ulong tcb_area_size;
extern ulong tcb_area_start_vaddr;

extern ulong get_per_cpu_area_start_vaddr(int cpu_id);
extern ulong get_my_cpu_area_start_vaddr();

extern ulong get_per_cpu_tcb_start_vaddr(int cpu_id);
extern ulong get_my_cpu_tcb_start_vaddr();

extern void init_mp();
extern void bringup_mp();
extern void release_mp_lock();

extern void ap_init_started();
extern void ap_init_done();


#endif
