#ifndef __ARCH_IA32_HAL_INCLUDE_CPU__
#define __ARCH_IA32_HAL_INCLUDE_CPU__


#include "common/include/data.h"


#define dec_per_cpu(type, name)         \
    int __##name##_per_cpu_index = -1;  \
    type __##name##_template
    
#define ext_per_cpu(type, name)         \
    extern int __##name##_per_cpu_index;\
    extern type __##name##_template
    
#define get_per_cpu(name)


#endif
