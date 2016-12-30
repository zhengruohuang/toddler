#include "common/include/asm.h"


.set noreorder
.set noat

.extern tlb_refill_handler
.extern cache_error_handler
.extern general_except_handler

.global int_entry_wrapper_begin
.global int_entry_wrapper_end


/*
 * The per-CPU entry template
 */
.align 12
int_entry_wrapper_begin:

/*
 * TLB refill
 */
tlb_refill_entry:
    // First of all, save current context (addr in $k0)
    sw ra, 124(k0)
    jal save_registers_except_ra
    nop
    
    // Prepare C arguments
    ori a0, k0, 0
    
    // Go to C to handle this!
    //  & switch stack!
    jal tlb_refill_handler
    ori sp, k1, 0
    
    // Restore context
    jal restore_registers_except_ra
    nop
    lw ra, 124(k0)
    
    // Done!
    eret

/*
 * Cache error
 */
.align 8
cache_error_entry:
    // Prepare C arguments
    ori a0, k0, 0
    
    // Go to C to handle this!
    //  & switch stack!
    jal cache_error_handler
    ori sp, k1, 0
    
    // Stop here!
    j .
    nop

/*
 * Other exceptions
 */
.align 7
general_except_entry:
    // First of all, save current context (addr in $k0)
    sw ra, 124(k0)
    jal save_registers_except_ra
    nop
    
    // Prepare C arguments
    ori a0, k0, 0
    
    // Go to C to handle this!
    //  & switch stack!
    jal general_except_handler
    ori sp, k1, 0
    
    // It seems like we can return directly... restore context
    jal restore_registers_except_ra
    nop
    lw ra, 124(k0)
    
    // Done!
    eret

/*
 * Save and restore registers
 */
save_registers_except_ra:
    //sw zero, 0(k0)
    //sw AT, 4(k0)
    sw v0, 8(k0)
    sw v1, 12(k0)
    sw a0, 16(k0)
    sw a1, 20(k0)
    sw a2, 24(k0)
    sw a3, 28(k0)
    sw t0, 32(k0)
    sw t1, 36(k0)
    sw t2, 40(k0)
    sw t3, 44(k0)
    sw t4, 48(k0)
    sw t5, 52(k0)
    sw t6, 56(k0)
    sw t7, 60(k0)
    sw t8, 64(k0)
    sw t9, 68(k0)
    sw s0, 72(k0)
    sw s1, 76(k0)
    sw s2, 80(k0)
    sw s3, 84(k0)
    sw s4, 88(k0)
    sw s5, 92(k0)
    sw s6, 96(k0)
    sw s7, 100(k0)
    //sw k0, 104(k0)
    //sw k1, 108(k0)
    sw gp, 112(k0)
    sw sp, 116(k0)
    sw fp, 120(k0)
    
    jr ra
    nop

restore_registers_except_ra:
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
    //lw s0, 72(k0)
    //lw s1, 76(k0)
    //lw s2, 80(k0)
    //lw s3, 84(k0)
    //lw s4, 88(k0)
    //lw s5, 92(k0)
    //lw s6, 96(k0)
    //lw s7, 100(k0)
    //lw k0, 104(k0)
    //lw k1, 108(k0)
    lw gp, 112(k0)
    lw sp, 116(k0)
    lw fp, 120(k0)
    
    jr ra
    nop

/*
 * Done
 */
int_entry_wrapper_end:
