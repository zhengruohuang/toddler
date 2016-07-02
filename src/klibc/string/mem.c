#include "common/include/data.h"
#include "klibc/include/string.h"


void memcpy(void *dest, void *src, size_t count)
{
    size_t i;
    
    unsigned char *s = (unsigned char *)src;
    unsigned char *d = (unsigned char *)dest;
    
    for (i = 0; i < count; i++) {
        *(d++) = *(s++);
    }
    
}

void memset(void *src, int value, size_t size)
{
    ulong i;
    unsigned char *ptr = (unsigned char *)src;
    
    for (i = 0; i < size; i++) {
        *(ptr++) = (unsigned char)value;
    }
    
}

void memzero(void *src, size_t size)
{
    ulong i;
    unsigned char *ptr = (unsigned char *)src;
    
    for (i = 0; i < size; i++) {
        *(ptr++) = (unsigned char)0x0;
    }
    
}

int memcmp(void *src1, void *src2, size_t len)
{
    unsigned char *cmp1 = (unsigned char *)src1;
    unsigned char *cmp2 = (unsigned char *)src2;
    
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
