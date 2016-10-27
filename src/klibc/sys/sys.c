#include "klibc/include/sys.h"


void sys_unreahable()
{
    while (1);
}

void sys_yield()
{
    syscall_yield();
}
