#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"


void msr_read(ulong reg, u64* value_out)
{
    u32* value32 = (u32*)value_out;
    u32 eax = 0;
    u32 edx = 0;
    
    __asm__ __volatile__
    (
        "rdmsr"
        : "=a" (eax), "=d" (edx)
        : "c" (reg)
    );
    
    value32[0] = eax;
    value32[1] = edx;
}

void msr_write(ulong reg, u64* value)
{
    u32* value32 = (u32*)value;
    u32 eax = value32[0];
    u32 edx = value32[1];
    
    __asm__ __volatile__
    (
        "wrmsr"
        :
        : "c" (reg), "a" (eax), "d" (edx)
    );
}

void msr_timestamp(u64* value)
{
    u32* value32 = (u32*)value;
    u32 eax = 0;
    u32 edx = 0;
    
    __asm__ __volatile__
    (
        "rdtsc"
        : "=a" (eax), "=d" (edx)
        :
    );
    
    value32[0] = eax;
    value32[1] = edx;
}
