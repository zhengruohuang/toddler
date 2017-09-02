#include "common/include/asm.h"

.global start
.global jump_to_real_mode
.global jump_to_hal

.section BOOTSTRAP, "ax"

start:
    b loader_entry
    nop
    
    b .
    nop

// r3 = paddr of boot params
// r4 = paddr of "jump_to_hal"
// r5 = paddr of real mode entry point
jump_to_real_mode:
    // Disable interrupts
    mfmsr r31
    rlwinm r31, r31, 0, 17, 15
    mtmsr r31

    // Set real mode entry point physical address
    mtspr srr0, r5

    // Disable address translation and interrupts
    //mfmsr r31
    //lis r30, ~0@h
    //ori r30, r30, ~(msr_ir | msr_dr | msr_ee)@l
    //and r31, r31, r30
    li r31, 0
    mtspr srr1, r31

    // Go
    sync
    isync
    rfi

// r3 = paddr of boot params
// r4 = vaddr of HAL entry
jump_to_hal:
    // Set HAL entry point
    mtspr srr0, r4
    
    // Set HAL stack
    subi sp, r4, 0x10
    
    // Set exception stack top
    //li r31, 0x100
    //mtsprg0 r31
    
    // This forces the usage of sprg0 as exception stack pointer
    //li sp, 0
    
    // Enable address translation
    mfmsr r31
    ori r31, r31, (msr_ir | msr_dr)@l
    mtspr srr1, r31
    
    // Go!
    sync
    isync
    rfi
