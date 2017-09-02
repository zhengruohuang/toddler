#include "common/include/asm.h"


.global restore_context_gpr


restore_context_gpr:
    // Restore GPRs
    //lwz r0, 0(r3)
    lwz r1, 4(r3)
    lwz r2, 8(r3)
    //lwz r3, 12(r3)
    //lwz r4, 16(r3)
    lwz r5, 20(r3)
    lwz r6, 24(r3)
    lwz r7, 28(r3)
    lwz r8, 32(r3)
    lwz r9, 36(r3)
    
    lwz r10, 40(r3)
    lwz r11, 44(r3)
    lwz r12, 48(r3)
    //lwz r13, 52(r3)
    lwz r14, 56(r3)
    lwz r15, 60(r3)
    lwz r16, 64(r3)
    lwz r17, 68(r3)
    lwz r18, 72(r3)
    lwz r19, 76(r3)
    
    lwz r20, 80(r3)
    lwz r21, 84(r3)
    lwz r22, 88(r3)
    lwz r23, 92(r3)
    lwz r24, 96(r3)
    lwz r25, 100(r3)
    lwz r26, 104(r3)
    lwz r27, 108(r3)
    lwz r28, 112(r3)
    lwz r29, 116(r3)
    
    lwz r30, 120(r3)
    lwz r31, 124(r3)
    
    // Other user regs: lr, cr, ctr, xer, msr, pc
    lwz r0, 128(r3)
    mtlr r0
    
    lwz r0, 132(r3)
    mtcr r0
    
    lwz r0, 136(r3)
    mtctr r0
    
    lwz r0, 140(r3)
    mtxer r0
    
    lwz r0, 144(r3)
    mtspr srr1, r0
    
    lwz r0, 148(r3)
    mtspr srr0, r0
    
    // Save the target value of r4 to sprg1 - scratch reg
    lwz r0, 16(r3)
    mtsprg1 r0
    
    // Restore r0 and r3
    lwz r0, 0(r3)
    lwz r3, 12(r3)
    
    // At this point we do not need to access memory any more
    // Now set up the new ASID
    //mtsr 0, r4
    addi r4, r4, 0x1
    mtsr 1, r4
    addi r4, r4, 0x1
    mtsr 2, r4
    addi r4, r4, 0x1
    mtsr 3, r4
    addi r4, r4, 0x1
    mtsr 4, r4
    addi r4, r4, 0x1
    mtsr 5, r4
    addi r4, r4, 0x1
    mtsr 6, r4
    addi r4, r4, 0x1
    mtsr 7, r4
    addi r4, r4, 0x1
    mtsr 8, r4
    addi r4, r4, 0x1
    mtsr 9, r4
    addi r4, r4, 0x1
    mtsr 10, r4
    addi r4, r4, 0x1
    mtsr 11, r4
    addi r4, r4, 0x1
    mtsr 12, r4
    addi r4, r4, 0x1
    mtsr 13, r4
    addi r4, r4, 0x1
    mtsr 14, r4
    //addi r4, r4, 0x1  // The last segment is reserved for HAL and kernel
    //mtsr 15, r4
    
    // Restore r4
    mfsprg1 r4
    
    // Done
    sync
    isync
    rfi
