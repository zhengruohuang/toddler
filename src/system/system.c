#include "common/include/data.h"


int a = 0;

void no_opt do_syscall(int a, int b, int c)
{
    __asm__ __volatile__
    (
        "movl   %%esp, %%ecx;"
        "lea   _sysenter_ret, %%edx;"

        "sysenter;"
        
        ".align 4;"
        "_sysenter_ret:;"
        :
        :
    );
}


void asmlinkage _start()
{
    do {
        __asm__ __volatile__
        (
            "pause;"
            :
            :
        );
        
        do_syscall(1, 2, 3);
    } while (1);
}
