#include "common/include/data.h"
#include "common/include/memory.h"


no_opt ulong ksys_get_tcb()
{
    unsigned long k1 = 0;
    
    // k1 - $27
    __asm__ __volatile__ (
        "move   %0, $27;"
        : "=r" (k1)
        :
    );
    
    // Convert k1 to unmapped address so we don't get TLB miss on this
    k1 = PHYS_TO_KCODE(k1);
    
    return k1;
}

no_opt int ksys_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2)
{
    int succeed = 0;
    unsigned long value1 = 0, value2 = 0;
    
    //    v0 -> $2
    // a0-a3 -> $4-$7
    __asm__ __volatile__ (
        "move $2, %3;"
        "move $4, %4;"
        "move $5, %5;"
        "syscall;"
        "move %0, $2;"
        "move %1, $4;"
        "move %2, $5;"
        : "=r" (succeed), "=r" (value1), "=r" (value2)
        : "r" (num), "r" (param1), "r" (param2)
        : "$2", "$4", "$5", "$6"
    );
    
    if (out1) {
        *out1 = value1;
    }
    
    if (out2) {
        *out2 = value2;
    }
    
    return 1;
}
