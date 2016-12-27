#include "common/include/asm.h"


.set noreorder

.extern tlb_refill_handler
.extern cache_error_handler
.extern general_except_handler

.global tlb_refill_entry
.global cache_error_entry
.global general_except_entry


/*
 * TLB refill
 */
.align 12
tlb_refill_entry:
    jal tlb_refill_handler
    nop

tlb_refill_stop:
    j tlb_refill_stop
    nop

/*
 * Cache error
 */
.align 8
cache_error_entry:
    j cache_error_handler
    nop

cache_error_stop:
    j cache_error_stop
    nop

/*
 * Other exceptions
 */
.align 7
general_except_entry:
    j general_except_handler
    nop

general_except_stop:
    j general_except_stop
    nop
