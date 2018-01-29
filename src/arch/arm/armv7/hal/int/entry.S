#include "hal/include/vecnum.h"


/* .set noreorder */

.extern int_handler_entry

.global int_entry_wrapper_begin
.global int_entry_wrapper_end


/*
 * The per-CPU entry template
 */
.align 12
int_entry_wrapper_begin:

int_vector_table:
    /* 0x00: reset ~ SVC */
    ldr pc, int_entry_jump_table
    
    /* 0x04: undefined instruction ~ UNDEF */
    ldr pc, int_entry_jump_table + 0x4
    
    /* 0x08: software interrupt ~ SVC */
    ldr pc, int_entry_jump_table + 0x8
    
    /* 0x0c: fetch abort ~ ABT */
    ldr pc, int_entry_jump_table + 0xc
    
    /* 0x10: data abort ~ ABT */
    ldr pc, int_entry_jump_table + 0x10
    
    /* 0x14: reserved ~ RESERVED */
    ldr pc, int_entry_jump_table + 0x14
    
    /* 0x18: IRQ ~ IRQ */
    ldr pc, int_entry_jump_table + 0x18
    
    /* 0x1c: FIQ ~ FIQ */
    ldr pc, int_entry_jump_table + 0x1c

int_entry_jump_table:
    .long reset_handler_entry
    .long undefined_handler_entry
    .long svc_handler_entry
    .long fetch_abort_handler_entry
    .long data_abort_handler_entry
    .long reserved_handler_entry
    .long irq_handler_entry
    .long fiq_handler_entry

/*
 * Done
 */
int_entry_wrapper_end:


/*
 * Helper macro
 */
.macro PREPARE
    cpsid aif
.endm
    
.macro PREPARE_PC pc_sub
    cpsid aif
    sub lr, lr, \pc_sub
.endm

.macro SAVE_REGS
    /* Disable all interrupts */
    cpsid iaf
    
    /* Save old PC */
    str lr, [sp, #0]
    
    /* Save old R0-R12 */
    add sp, sp, #4
    stmea sp, {r0-r12}^
    
    /* Save PPSR (old CPSR) */
    mrs r0, SPSR
    str r0, [sp, #60]
    
    /* Save old SP (R13) and LR (R14) */
    add r0, sp, #52
    stmea r0, {sp, lr}^
    nop
    
    /* Restore current SP */
    sub sp, sp, #4
.endm

.macro ENTER_C vector
    /* Prepare C arguments */
    mov r0, \vector
    mov r1, sp
    
    /* Prepare SP for system mode -> r2 */
    mov r2, sp
    sub r2, r2, #16     /* Set a 16B guard */
    
    /* Switch to system mode */
    cpsid iaf, #0x1f
    
    /* Set SP */
    mov sp, r2
    
    /* Call the C entry */
    bl int_handler_entry
    
    /* Should never reach here */
/*     b failed */
.endm

.macro RETURN mode
    /* Switch to SVC mode */
    cpsid iaf, \mode
    
    /* Load SPSR */
    ldr r0, [sp, #64]
    msr SPSR, r0
    
    /* Restore user/system SP (R13) and LR (R14) */
    add sp, sp, #4
    add r0, sp, #52
    ldmfd r0, {sp, lr}^
    
    /* Restore user/system R0-R12 */
    ldmfd sp, {r0-r12}^
    nop
    
    /* Restore PC and trigger SPSR->CPSR */
    sub sp, sp, #4
    ldmfd sp, {pc}^

    /* Should never reach here */
    b failed

.endm


/*
 * Handler entries
 */
reset_handler_entry:
    PREPARE
    SAVE_REGS
    ENTER_C #INT_VECTOR_RESET
    RETURN #0x13

undefined_handler_entry:
    PREPARE
    SAVE_REGS
    ENTER_C #INT_VECTOR_UNDEFINED
    RETURN #0x1b

svc_handler_entry:
    PREPARE
    SAVE_REGS
    ENTER_C #INT_VECTOR_SVC
    RETURN #0x13

fetch_abort_handler_entry:
    PREPARE_PC #4
    SAVE_REGS
    ENTER_C #INT_VECTOR_FETCH
    RETURN #0x17

data_abort_handler_entry:
    PREPARE_PC #8
    SAVE_REGS
    ENTER_C #INT_VECTOR_DATA
    RETURN #0x17

reserved_handler_entry:
    PREPARE
    SAVE_REGS
    ENTER_C #INT_VECTOR_RESERVED
    RETURN #0x13

irq_handler_entry:
    PREPARE_PC #4
    SAVE_REGS
    ENTER_C #INT_VECTOR_IRQ
    RETURN #0x12

fiq_handler_entry:
    PREPARE_PC #4
    SAVE_REGS
    ENTER_C #INT_VECTOR_FIQ
    RETURN #0x11


/*
 * Illegal return
 */
failed:
    b .
    nop
