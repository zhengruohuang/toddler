#include "common/include/data.h"


int a = 0;


void asmlinkage _start()
{
    do {
        __asm__ __volatile__
        (
            //"xchgw  %%bx, %%bx;"
            "pause;"
            :
            :
        );
    } while (1);
}
