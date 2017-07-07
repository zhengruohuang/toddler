#include "common/include/data.h"


void asmlinkage no_opt hal_entry()
{
    __asm__ __volatile__
    (
        "xor 3, 3, 3;"
        "addi 3, 3, 0xbe;"
    );
    while (1);
    
    while (1);
}
