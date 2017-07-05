#include "common/include/asm.h"

.global start

.section BOOTSTRAP, "ax"

start:
    //lis r4, ofw_cif@ha
    //addi r4, r4, ofw_cif@l
    //stw r5, 0(r4)

    addi r3, r5, 0
    bl bootstrap
    nop
    
    b .
    nop
