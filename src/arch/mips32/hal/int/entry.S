#include "common/include/asm.h"


.set noreorder
.set noat

.extern tlb_refill_handler
.extern cache_error_handler
.extern general_except_handler

.global int_entry_template_begin
.global int_entry_template_end


/*
 * Common entries
 */
common_tlb_refill_entry:
    eret
    
common_general_except_entry:
    // First of all, save current context (addr in $k0)
    sw AT, 0(k0)
    sw v0, 4(k0)
    sw v1, 8(k0)
    
    // Go to C to handle this!
    jal general_except_handler
    nop
    
    // It seems like we can return directly... restore context
    lw AT, 0(k0)
    lw v0, 4(k0)
    lw v1, 8(k0)
    
    // Done!
    eret
 
/*
 * The per-CPU entry template
 */
.align 12
int_entry_template_begin:

/*
 * TLB refill
 */
tlb_refill_entry:
    la k0, is_cur_context_kernel
    lw k0, 0(k0)
    
    la k1, cur_context_page_dir_ptr
    lw k1, 0(k1)
    
    j common_tlb_refill_entry
    nop

/*
 * Cache error
 */
.align 8
cache_error_entry:
    jal cache_error_handler
    nop

/*
 * Other exceptions
 */
.align 7
general_except_entry:
    la k0, cur_context_save
    j common_general_except_entry
    nop
    
/*    la $t0, base     // load the address of "base"
    la $t1, count    // load the address of "count"
    lw $t2, 0($t0)   // load the data at location "base"
    sw $t2, 0($t1)   // store that data at location "count"*/

/*
 * Vector entry
 */
.align 7
vector_int_entry:
    nop

vector_int_stop:
    j vector_int_stop
    nop

/*
 * Per-CPU variables
 */
is_cur_context_kernel:      .fill 4
cur_context_page_dir_ptr:   .fill 4
cur_context_save:           .fill 4 * 32

/*
 * Done
 */
int_entry_template_end:
