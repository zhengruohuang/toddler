.global restore_context_gpr

restore_context_gpr:
    /* Switch to SVC mode */
    cpsid aif, #0x13
    
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
    b .
    nop
