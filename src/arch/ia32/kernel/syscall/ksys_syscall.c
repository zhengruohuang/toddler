#include "common/include/data.h"


#define GEN_INT_INSTR(vec) "int $" #vec ";"


no_opt ulong ksys_get_tcb()
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
    
    return addr;
}

no_opt int ksys_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2)
{
    int succeed = 0;
    unsigned long value1 = 0, value2 = 0;
    
    __asm__ __volatile__
    (
        GEN_INT_INSTR(0x7f)
        : "=a" (succeed), "=S" (value1), "=D" (value2)
        : "S"(num), "D" (param1), "a" (param2)
        : "memory"
    );
    
    if (out1) {
        *out1 = value1;
    }
    
    if (out2) {
        *out2 = value2;
    }
    
    return succeed;
}
