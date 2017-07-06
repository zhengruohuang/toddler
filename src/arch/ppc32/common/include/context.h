#ifndef __ARCH_PPC32_COMMON_INCLUDE_CONTEXT__
#define __ARCH_PPC32_COMMON_INCLUDE_CONTEXT__


#include "common/include/data.h"


/*
 * Context
 */
struct context {
    u32 zero;
    u32 at;
    u32 v0, v1;
    u32 a0, a1, a2, a3;
    u32 t0, t1, t2, t3, t4, t5, t6, t7;
    u32 s0, s1, s2, s3, s4, s5, s6, s7;
    u32 t8, t9;
    u32 k0, k1;
    u32 gp;
    u32 sp;
    u32 fp;
    u32 ra;
    
    u32 pc;
} packedstruct;


#endif
