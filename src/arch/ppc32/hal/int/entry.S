#include "common/include/asm.h"
#include "hal/include/vecnum.h"


.extern general_except_handler

.global __int_entry_wrapper_begin
.global __int_entry_wrapper_end


/*
 * Interrupt handler templates
 * The first vector is reserved
 * 
 * Registers
 *   sprg0 - Translated stack top
 *   sprg1 - Scratch
 *   sprg2 - Vector
 *   sprg3 - Reserved
 */
.align 8
__int_entry_wrapper_begin:
reserved0_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
system_reset_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
machine_check_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
dsi_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DSI
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
isi_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_ISI
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
external_interrupt_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_EXTERNAL
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
alignment_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
program_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_PROGRAM
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
fp_unavail_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
dec_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DECREMENTER
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
reserved1_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
reserved2_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
syscall_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_SYSCALL
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
trace_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
fp_assist_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
perf_mon_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
vector_unvail_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
itlb_miss_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
dtlb_miss_load_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
dtlb_miss_store_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
breakpoint_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
system_mgmt_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop

.align 8
vector_assist_entry:
    mtsprg1 r0
    li r0, INT_VECTOR_DUMMY
    mtsprg2 r0
    mfsprg1 r0
    
    b save_registers
    nop


/*
 * Save registers
 */
.align 8
save_registers:
    // Re-enable address translation
    mtsprg1 r31
    
    mfmsr r31
    ori r31, r31, (msr_ir | msr_dr)@l
    mtmsr r31
    sync
    isync
    
    mfsprg1 r31

    // Set up stack
    mtsprg1 sp
    mfsprg0 sp
    
    // Save GPRs
    stw r0, 0(sp)
    stw r1, 4(sp)
    stw r2, 8(sp)
    stw r3, 12(sp)
    stw r4, 16(sp)
    stw r5, 20(sp)
    stw r6, 24(sp)
    stw r7, 28(sp)
    stw r8, 32(sp)
    stw r9, 36(sp)
    
    stw r10, 40(sp)
    stw r11, 44(sp)
    stw r12, 48(sp)
    stw r13, 52(sp)
    stw r14, 56(sp)
    stw r15, 60(sp)
    stw r16, 64(sp)
    stw r17, 68(sp)
    stw r18, 72(sp)
    stw r19, 76(sp)
    
    stw r20, 80(sp)
    stw r21, 84(sp)
    stw r22, 88(sp)
    stw r23, 92(sp)
    stw r24, 96(sp)
    stw r25, 100(sp)
    stw r26, 104(sp)
    stw r27, 108(sp)
    stw r28, 112(sp)
    stw r29, 116(sp)
    
    stw r30, 120(sp)
    stw r31, 124(sp)
    
    // User SP
    mfsprg1 r0
    stw r0, 4(sp)
    
    // Other user regs: lr, cr, ctr, xer, msr, pc
    mflr r0
    stw r0, 128(sp)
    
    mfcr r0
    stw r0, 132(sp)
    
    mfctr r0
    stw r0, 136(sp)
    
    mfxer r0
    stw r0, 140(sp)
    
    mfspr r0, srr1
    stw r0, 144(sp)
    
    mfspr r0, srr0
    stw r0, 148(sp)
    
    // Set up the new stack
    subi sp, sp, 0x10
    
    // Set up the new ASID
    lis r4, 0x40000000@ha
    
    mtsr 0, r4
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
    addi r4, r4, 0x1
    mtsr 15, r4
    
    // Go to handler entry
    lis r31, general_except_handler@ha
    addi r31, r31, general_except_handler@l
    
    mtlr r31
    blr
    
/*
 * Done
 */
__int_entry_wrapper_end:
