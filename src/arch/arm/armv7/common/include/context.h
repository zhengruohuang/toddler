#ifndef __ARCH_ARMV7_COMMON_INCLUDE_CONTEXT__
#define __ARCH_ARMV7_COMMON_INCLUDE_CONTEXT__


#include "common/include/data.h"


/*
 * Context
 */
struct context {
    u32 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    u32 r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    u32 r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
    u32 r30, r31;
    u32 lr, cr, ctr, xer, msr, pc;
} packedstruct;


#endif
