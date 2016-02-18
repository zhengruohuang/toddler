#include "common/include/data.h"


void io_out32(ulong port, ulong value)
{
    __asm__ __volatile__
    (
        "outl   %%eax, %%dx;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        :
        : "d" (port), "a" (value)
    );
}

void io_out16(ulong port, ulong value)
{
    __asm__ __volatile__
    (
        "outw   %%ax, %%dx;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        :
        : "d" (port), "a" (value & 0xffff)
    );
}

void io_out8(ulong port, ulong value)
{
    __asm__ __volatile__
    (
        "outb   %%al, %%dx;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        :
        : "d" (port), "a" (value & 0xff)
    );
}

ulong io_in32(ulong port)
{
    ulong value = 0;
    
    __asm__ __volatile__
    (
        "inl   %%dx, %%eax;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        : "=a" (value)
        : "d" (port)
    );
    
    return value;
}

ulong io_in16(ulong port)
{
    ulong value = 0;
    
    __asm__ __volatile__
    (
        "inw   %%dx, %%ax;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        : "=a" (value)
        : "d" (port)
    );
    
    return value & 0xffff;
}

ulong io_in8(ulong port)
{
    ulong value = 0;
    
    __asm__ __volatile__
    (
        "inb   %%dx, %%al;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        "nop;"
        : "=a" (value)
        : "d" (port)
    );
    
    return value & 0xff;
}
