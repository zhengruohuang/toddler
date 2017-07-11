#include "common/include/asm.h"


.global __int_entry_wrapper_begin
.global __int_entry_wrapper_end


/*
 * Interrupt handler templates
 * The first vector is reserved
 */
.align 8
__int_entry_wrapper_begin:
reserved0_entry:
    b .
    nop

.align 8
system_reset_entry:
    nop

.align 8
machine_check_entry:
    nop

.align 8
dsi_entry:
    nop

.align 8
isi_entry:
    nop

.align 8
external_interrupt_entry:
    nop

.align 8
alignment_entry:
    nop

.align 8
program_entry:
    nop

.align 8
fp_unavail_entry:
    nop

.align 8
dec_entry:
    nop

.align 8
reserved1_entry:
    nop

.align 8
reserved2_entry:
    nop

.align 8
syscall_entry:
    nop

.align 8
trace_entry:
    nop

.align 8
fp_assist_entry:
    nop

.align 8
perf_mon_entry:
    nop

.align 8
vector_unvail_entry:
    nop

.align 8
itlb_miss_entry:
    nop

.align 8
dtlb_miss_load_entry:
    nop

.align 8
dtlb_miss_store_entry:
    nop

.align 8
breakpoint_entry:
    nop

.align 8
system_mgmt_entry:
    nop

.align 8
vector_assist_entry:
    nop


/*
 * Done
 */
__int_entry_wrapper_end:
