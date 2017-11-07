#ifndef __ARCH_MIPS_COMMON_INCLUDE_CONTEXT__
#define __ARCH_MIPS_COMMON_INCLUDE_CONTEXT__


#include "common/include/data.h"


/*
 * Context
 */
struct context {
    ulong zero;
    ulong at;
    ulong v0, v1;
    ulong a0, a1, a2, a3;
    ulong t0, t1, t2, t3, t4, t5, t6, t7;
    ulong s0, s1, s2, s3, s4, s5, s6, s7;
    ulong t8, t9;
    ulong k0, k1;
    ulong gp;
    ulong sp;
    ulong fp;
    ulong ra;
    
    ulong pc;
    ulong delay_slot;
} packedstruct;


#endif
