#include "common/include/asm.h"


.set noreorder
.set noat

.extern tlb_refill_handler
.extern cache_error_handler
.extern general_except_handler

.extern per_cpu_context_ptr_base

.global int_entry_wrapper_begin
.global int_entry_wrapper_end


/*
 * Interrupt handler entry template
 */
.align 12
int_entry_wrapper_begin:

/*
 * TLB refill
 */
tlb_refill_entry:
    xor k0, k0, k0
    xor k1, k1, k1
    
    // Figure out addr of the context struct of this cpu
    dla k0, per_cpu_context_ptr_base
    ld k0, 0(k0)
    mfc0 k1, $15, 1     // Read EBbase - CP0 reg 15 sel 1
    andi k1, k1, 0x3ff  // Get CPU ID
    sll k1, k1, 3       // Calculate offset - cpu_id * 8
    addu k0, k0, k1     // Addr of context - base + offset
    ld k0, 0(k0)
    
    // First of all, save current context (addr in $k0)
    sd ra, 248(k0)
    jal save_registers_except_ra
    nop
    
    // Prepare C arguments
    move a0, k0
    
    // Go to C to handle this!
    //  & switch stack!
    ld sp, 272(k0)
    jal tlb_refill_handler
    nop
    
    // Restore context
    jal restore_registers_except_ra
    nop
    ld ra, 248(k0)
    
    // Done!
    eret
    nop

/*
 * 64-bit TLB refill
 */
.align 7
tlb_refill_entry64:
    xor k0, k0, k0
    xor k1, k1, k1
    
    // Figure out addr of the context struct of this cpu
    dla k0, per_cpu_context_ptr_base
    ld k0, 0(k0)
    mfc0 k1, $15, 1     // Read EBbase - CP0 reg 15 sel 1
    andi k1, k1, 0x3ff  // Get CPU ID
    sll k1, k1, 3       // Calculate offset - cpu_id * 8
    addu k0, k0, k1     // Addr of context - base + offset
    ld k0, 0(k0)
    
    // First of all, save current context (addr in $k0)
    sd ra, 248(k0)
    jal save_registers_except_ra
    nop
    
    // Prepare C arguments
    move a0, k0
    
    // Go to C to handle this!
    //  & switch stack!
    ld sp, 272(k0)
    jal tlb_refill_handler
    nop
    
    // Restore context
    jal restore_registers_except_ra
    nop
    ld ra, 248(k0)
    
    // Done!
    eret
    nop


/*
 * Cache error
 */
.align 7
cache_error_entry:
    xor k0, k0, k0
    xor k1, k1, k1
    
    // Figure out addr of the context struct of this cpu
    dla k0, per_cpu_context_ptr_base
    ld k0, 0(k0)
    mfc0 k1, $15, 1     // Read EBbase - CP0 reg 15 sel 1
    andi k1, k1, 0x3ff  // Get CPU ID
    sll k1, k1, 3       // Calculate offset - cpu_id * 8
    addu k0, k0, k1     // Addr of context - base + offset
    ld k0, 0(k0)
    
    // Prepare C arguments
    move a0, k0
    
    // Go to C to handle this!
    //  & switch stack!
    ld sp, 280(k0)
    jal cache_error_handler
    nop
    
    // Stop here!
    j .
    nop

/*
 * Other exceptions
 */
.align 7
general_except_entry:
    xor k0, k0, k0
    xor k1, k1, k1
    
    // Figure out addr of the context struct of this cpu
    dla k0, per_cpu_context_ptr_base
    ld k0, 0(k0)
    mfc0 k1, $15, 1     // Read EBbase - CP0 reg 15 sel 1
    andi k1, k1, 0x3ff  // Get CPU ID
    sll k1, k1, 3       // Calculate offset - cpu_id * 8
    addu k0, k0, k1     // Addr of context - base + offset
    ld k0, 0(k0)
    
    // First of all, save current context (addr in $k0)
    sd ra, 248(k0)
    jal save_registers_except_ra
    nop
    
    // Prepare C arguments
    move a0, k0
    
    // Go to C to handle this!
    //  & sditch stack!
    ld sp, 280(k0)
    jal general_except_handler
    nop
    
    // It seems like we can return directly... restore context
    jal restore_registers_except_ra
    nop
    ld ra, 248(k0)
    
    // Done!
    eret
    nop

/*
 * Save and restore registers
 */
 .align 7
save_registers_except_ra:
    //sd zero, 0(k0)
    sd AT, 8(k0)
    sd v0, 16(k0)
    sd v1, 24(k0)
    sd a0, 32(k0)
    sd a1, 40(k0)
    sd a2, 48(k0)
    sd a3, 56(k0)
    sd t0, 64(k0)
    sd t1, 72(k0)
    sd t2, 80(k0)
    sd t3, 88(k0)
    sd t4, 96(k0)
    sd t5, 104(k0)
    sd t6, 112(k0)
    sd t7, 120(k0)
    sd t8, 128(k0)
    sd t9, 136(k0)
    sd s0, 144(k0)
    sd s1, 152(k0)
    sd s2, 160(k0)
    sd s3, 168(k0)
    sd s4, 176(k0)
    sd s5, 184(k0)
    sd s6, 192(k0)
    sd s7, 200(k0)
    //sd k0, 208(k0)
    //sd k1, 216(k0)
    sd gp, 224(k0)
    sd sp, 232(k0)
    sd fp, 240(k0)
    
    jr ra
    nop

restore_registers_except_ra:
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
    
    jr ra
    nop

/*
 * Done
 */
int_entry_wrapper_end:
