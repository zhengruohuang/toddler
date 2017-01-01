#include "common/include/asm.h"


.set noreorder
.set noat

.global restore_context_gpr

restore_context_gpr:
    //lw zero, 0(k0)
    lw AT, 4(k0)
    lw v0, 8(k0)
    lw v1, 12(k0)
    lw a0, 16(k0)
    lw a1, 20(k0)
    lw a2, 24(k0)
    lw a3, 28(k0)
    lw t0, 32(k0)
    lw t1, 36(k0)
    lw t2, 40(k0)
    lw t3, 44(k0)
    lw t4, 48(k0)
    lw t5, 52(k0)
    lw t6, 56(k0)
    lw t7, 60(k0)
    lw t8, 64(k0)
    lw t9, 68(k0)
    lw s0, 72(k0)
    lw s1, 76(k0)
    lw s2, 80(k0)
    lw s3, 84(k0)
    lw s4, 88(k0)
    lw s5, 92(k0)
    lw s6, 96(k0)
    lw s7, 100(k0)
    //lw k0, 104(k0)
    //lw k1, 108(k0)
    lw gp, 112(k0)
    lw sp, 116(k0)
    lw fp, 120(k0)
    lw ra, 124(k0)
    
    eret
    nop
