#include "common/include/data.h"
#include "common/include/proc.h"
#include "common/include/reg.h"
#include "klibc/include/stdio.h"


no_opt struct thread_control_block *get_tcb()
{
    unsigned long tcb = 0;
    read_software_thread_id(tcb);
    
    return (struct thread_control_block *)tcb;
}

no_opt int do_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2)
{
    int success = 0;
    unsigned long value1 = 0, value2 = 0;
    
    __asm__ __volatile__ (
        "mov r0, %[num];"
        "mov r1, %[p1];"
        "mov r2, %[p2];"
        "swi 0;"
        "mov %[suc], r0;"
        "mov %[v1], r1;"
        "mov %[v2], r2;"
        : [suc] "=r" (success), [v1] "=r" (value1), [v2] "=r" (value2)
        : [num] "r" (num), [p1] "r" (param1), [p2] "r" (param2)
        : "r0", "r1", "r2", "memory"
    );
    
    if (out1) *out1 = value1;
    if (out2) *out2 = value2;
    
    return 1;
}
