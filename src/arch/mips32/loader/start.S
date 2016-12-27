#include "common/include/asm.h"
#include "common/include/memlayout.h"


LEAF(_start)
    .set    noreorder
    
    // Disable interrupts
    mtc0    zero, CP0_STATUS
    
    // Disable watch exception
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI
    
    // Disable kernel mode cache
    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    // Set up stack
    li      sp, SEG_LOW_DIRECT + LOADER_STACK_TOP - 0x10
    
    // Jump to main
    jal     main
    nop

    // Should never reach here
stop:
    j       stop
    nop

END(_start)
