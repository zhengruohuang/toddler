.section ".text.boot"


.global _start
.global _start_ap


// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
_start:
    // Go to C!
/*     ldr sp, =0x8000         // Setup the stack */
    mov x0, #0xbeef
    b .
    nop
    b loader_entry          // Call loader_entry
 
    // Should never reach here
    b .
    nop


_start_ap:
/*     ldr sp, =0x8000         // Setup the stack */
    b loader_ap_entry       // Call loader_entry
    
    // Should never reach here
    b .
    nop
