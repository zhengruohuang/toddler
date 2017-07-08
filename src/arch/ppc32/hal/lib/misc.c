#include "common/include/data.h"
#include "hal/include/print.h"
//#include "hal/include/int.h"


void no_opt halt()
{
    //disable_local_int();
    
    __asm__ __volatile__ (
        "xor 3, 3, 3;"
        "addi 3, 3, 0xbe;"
        
        "xor 4, 4, 4;"
        "addi 4, 4, 0xef;"
    );
    
    while (1);
}

void disp_src_info(char *file, char *base, int line)
{
    //kprintf("[SRC] File: %s, Base: %s, Line: %d\n", file, base, line);
}
