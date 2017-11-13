#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/reg.h"


no_opt ulong ksys_get_tcb()
{
    unsigned long k1 = 0;
    
    // k1 - $27
    read_k1(k1);
    
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
        "move $2, %[num];"
        "move $4, %[p1];"
        "move $5, %[p2];"
        "syscall;"
        "nop;"
        "move %[suc], $2;"
        "move %[val1], $4;"
        "move %[val2], $5;"
        : [suc] "=r" (succeed), [val1] "=r" (value1), [val2] "=r" (value2)
        : [num] "r" (num), [p1] "r" (param1), [p2] "r" (param2)
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
