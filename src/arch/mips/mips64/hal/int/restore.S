#include "common/include/asm.h"


.set noreorder
.set noat

.global restore_context_gpr

restore_context_gpr:
    //ld zero, 0(k0)
    ld AT, 8(k0)
    ld v0, 16(k0)
    ld v1, 24(k0)
    ld a0, 32(k0)
    ld a1, 40(k0)
    ld a2, 48(k0)
    ld a3, 56(k0)
    ld t0, 64(k0)
    ld t1, 72(k0)
    ld t2, 80(k0)
    ld t3, 88(k0)
    ld t4, 96(k0)
    ld t5, 104(k0)
    ld t6, 112(k0)
    ld t7, 120(k0)
    ld t8, 128(k0)
    ld t9, 136(k0)
    ld s0, 144(k0)
    ld s1, 152(k0)
    ld s2, 160(k0)
    ld s3, 168(k0)
    ld s4, 176(k0)
    ld s5, 184(k0)
    ld s6, 192(k0)
    ld s7, 200(k0)
    //ld k0, 208(k0)
    //ld k1, 216(k0)
    ld gp, 224(k0)
    ld sp, 232(k0)
    ld fp, 240(k0)
    ld ra, 248(k0)
    
    eret
    nop
