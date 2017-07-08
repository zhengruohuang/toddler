#include "common/include/asm.h"

.global start

.section BOOTSTRAP, "ax"

start:
    //xor r3, r3, r3
    //addi r3, r3, 0xbe
    
    //xor r4, r4, r4
    //addi r4, r4, 0xef
    
    b hal_entry
    nop
    
    b .
    nop
    
