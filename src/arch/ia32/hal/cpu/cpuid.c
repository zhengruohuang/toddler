#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"


static int cpuid_supported = 0;
//static int vendor_type = -1;


void init_cpuid()
{
    ulong val, ret;
    
    __asm__ __volatile__
    (
        "pushf\n"                    // read flags
        "popl %[ret]\n"
        "movl %[ret], %[val]\n"
        
        "btcl $21, %[val]\n"         // swap the ID bit
        
        "pushl %[val]\n"             // propagate the change into flags
        "popf\n"
        "pushf\n"
        "popl %[val]\n"
        
        "andl $(1 << 21), %[ret]\n"  // interrested only in ID bit
        "andl $(1 << 21), %[val]\n"
        "xorl %[val], %[ret]\n"
        : [ret] "=r" (ret), [val] "=r" (val)
    );
    
    cpuid_supported = (int)ret;
}


int cpuid(struct cpuid_reg *reg)
{
    if (!cpuid_supported) {
        return 0;
    }
    
    __asm__ __volatile__
    (
        "cpuid;"
        : "=a" (reg->a), "=b" (reg->b), "=c" (reg->c), "=d" (reg->d)
        : "a" (reg->a), "b" (reg->b), "c" (reg->c), "d" (reg->d)
    );
    
    return 1;
}
