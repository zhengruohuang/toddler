#include "common/include/data.h"
#include "common/include/proc.h"
#include "klibc/include/stdio.h"


no_opt struct thread_control_block *get_tcb()
{
    unsigned long tcb = 0;
    
    return (struct thread_control_block *)tcb;
}

no_opt int do_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2)
{
    return 0;
}
