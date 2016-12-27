#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/proc.h"
#include "hal/include/print.h"


int get_cpu_id()
{
    u32 cpu_num = 0;
    
    __asm__ __volatile__ (
        "rdhwr  %0, $0;"
        :
        : "r" (cpu_num)
    );
    
    return (int)cpu_num;
}

void init_mp()
{
    kprintf("Initializing multiprocessor support\n");
}
