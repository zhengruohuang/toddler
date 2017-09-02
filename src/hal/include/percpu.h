#ifndef __HAL_INCLUDE_PERCPU__
#define __HAL_INCLUDE_PERCPU__


#include "common/include/data.h"


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


#endif
