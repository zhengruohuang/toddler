#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"


no_opt struct thread_control_block *get_tcb()
{
    unsigned long addr = 0;
    

    __asm__ __volatile__
    (
        "xorl   %%esi, %%esi;"
        "movl   %%gs:(%%esi), %%edi;"
        : "=D" (addr)
        :
        : "%esi"
    );
    
    return (struct thread_control_block *)addr;
}

no_opt int do_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2)
{
    int succeed = 0;
    unsigned long value1 = 0, value2 = 0;
    
    __asm__ __volatile__
    (
        "movl   %%esp, %%ecx;"
        "lea   _sysenter_ret, %%edx;"
        
        "sysenter;"
        
        ".align 4;"
        "_sysenter_ret:;"
        : "=a" (succeed), "=S" (value1), "=D" (value2)
        : "S"(num), "D" (param1), "a" (param2)
        : "%ecx", "%edx"
    );
    
    if (out1) {
        *out1 = value1;
    }
    
    if (out2) {
        *out2 = value2;
    }
    
    return 1;
}
