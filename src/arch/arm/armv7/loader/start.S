.equ SCTLR_ENABLE_DATA_CACHE,           0x4
.equ SCTLR_ENABLE_BRANCH_PREDICTION,    0x800
.equ SCTLR_ENABLE_INSTRUCTION_CACHE,    0x1000


.section ".text.boot"


.global _start
.global _start_ap


// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
_start:
    // Take care of multicore
    //mrc p15, 0, r0, c0, c0, 5     // R0 = Multiprocessor Affinity Register (MPIDR)
    //ands r0, #3                   // R0 = CPU ID (Bits 0..1)
    //bne core_wait                 // IF (CPU ID != 0) Branch To Infinite Loop (Core ID 1..3)
    
    //mov r0, #0x40000000
    //b .
    //nop
    
/*    // Enable FP
    mrc p15, 0, r1, c1, c0, #2      // r1 = Access Control Register
    orr r1, r1, #(0xf << 20)        // enable full access for p10,11
    mcr p15, #0, r1, c1, c0, #2     // ccess Control Register = r1
    mov r1, #0                      // flush prefetch buffer because of FMXR below
    mcr p15, #0, r1, c7, c5, #4     // and CP 10 & 11 were only just enabled
    mov r0, #0x40000000             // Enable VFP itself
    fmxr fpexc, r0                  // FPEXC = r0
    
    // Enable caches
    mrc p15, 0, r0, c1, c0, 0                   // R0 = System Control Register
    orr r0, #SCTLR_ENABLE_BRANCH_PREDICTION     // Enable caches and branch prediction
    orr r0, #SCTLR_ENABLE_DATA_CACHE
    orr r0, #SCTLR_ENABLE_INSTRUCTION_CACHE
    mcr p15, 0, r0, c1, c0, 0                   // System Control Register = R0*/
    
    // Caches shouldn't be enabled at this point
    
    // Go to C!
    ldr sp, =0x8000         // Setup the stack
    b loader_entry          // Call loader_entry
 
    // Should never reach here
    b .
    nop


_start_ap:
    ldr sp, =0x8000         // Setup the stack
    b loader_ap_entry       // Call loader_entry
    
    // Should never reach here
    b .
    nop
