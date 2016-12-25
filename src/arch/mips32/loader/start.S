#include "common/include/asm.h"
#include "common/include/memlayout.h"


LEAF(_start)
    .set    mips2
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
    
    // Set up stack @ 1MB
    li      sp, SEG_LOW_DIRECT + 0x100000 - 16
    
    // Jump to main
    jal     main
    nop

    // Should never reach here
stop:
    j       stop
    nop

END(_start)
