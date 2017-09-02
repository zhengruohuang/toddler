#include "common/include/asm.h"
#include "common/include/memlayout.h"


LEAF(_start)
    .set    noreorder
    
    // Disable watch exception
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI
    
    // Disable kseg0 cache
    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    // Set up stack
    li      sp, SEG_LOW_CACHED + LOADER_STACK_TOP - 0x10
    
    // Jump to loader entry
    jal     loader_entry
    nop

    // Should never reach here
stop:
    j       stop
    nop

END(_start)
