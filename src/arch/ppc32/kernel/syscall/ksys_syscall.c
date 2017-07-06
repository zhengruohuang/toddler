#include "common/include/data.h"
#include "common/include/memory.h"


no_opt ulong ksys_get_tcb()
{
    unsigned long k1 = 0;
    
    return k1;
}

no_opt int ksys_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2)
{
    int success = 0;
    unsigned long value1 = 0, value2 = 0;
    
    if (out1) {
        *out1 = value1;
    }
    
    if (out2) {
        *out2 = value2;
    }
    
    return success;
}
