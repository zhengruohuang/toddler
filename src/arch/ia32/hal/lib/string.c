#include "common/include/data.h"
#include "hal/include/print.h"


void memcpy(void *src, void *dest, size_t count)
{
    ulong i;
    
    ulong source = (ulong)src;
    ulong destination = (ulong)dest;
    
    for (i = 0; i < count; i++) {
        if (!(i % 1024)) {
            kprintf(".");
        }
        
        __asm__ __volatile__
        (
            "movb   (%%esi), %%al;"
            "movb   %%al, (%%edi);"
            :
            : "S" (source + i), "D" (destination + i)
        );
    }
    
}

void memset(void *src, int value, size_t size)
{
    ulong i;
    u8 *ptr = (u8 *)src;
    
    for (i = 0; i < size; i++) {
        *(ptr++) = (u8)value;
    }
    
}

void memzero(void *src, size_t size)
{
    ulong i;
    u8 *ptr = (u8 *)src;
    
    for (i = 0; i < size; i++) {
        *(ptr++) = (u8)0x0;
    }
    
}

int memcmp(void *src1, void *src2, size_t len)
{
    u8 *cmp1 = (u8 *)src1;
    u8 *cmp2 = (u8 *)src2;
    
    ulong i;
    int result = 0;
    
    for (i = 0; i < len; i++) {
        if (*(cmp1 + i) > *(cmp2 + i)) {
            return 1;
        } else if (*(cmp1 + i) < *(cmp2 + i)) {
            return -1;
        }
    }
    
    return result;
}
