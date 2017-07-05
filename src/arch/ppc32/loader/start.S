#include "common/include/asm.h"

.global start

.section BOOTSTRAP, "ax"

start:
    b loader_entry
    nop
    
    b .
    nop
