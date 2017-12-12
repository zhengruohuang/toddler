#include "common/include/data.h"


/*
 * String
 */
int strcmp(char *s1, char *s2)
{
    int result = 0;
    
    while (*s1 || *s2) {
        if (!s1 && !s2) {
            return 0;
        }
        
        if (*s1 != *s2) {
            return *s1 > *s2 ? 1 : -1;
        }
        
        s1++;
        s2++;
    }
    
    return result;
}

int strlen(const char *s)
{
    int len = 1;
    
    while (*s++) {
        len++;
    }
    
    return len;
}


/*
 * Memory
 */
void memcpy(void *dest, void *src, size_t count)
{
    ulong i;
    
    u8 *s = (u8 *)src;
    u8 *d = (u8 *)dest;
    
    for (i = 0; i < count; i++) {
        d[i] = s[i];
    }
    
}

void memset(void *src, int value, size_t size)
{
    ulong i;
    u8 *ptr = (u8 *)src;
    
    for (i = 0; i < size; i++) {
        ptr[i] = (u8)value;
    }
    
}

void memzero(void *src, size_t size)
{
    ulong i;
    u8 *ptr = (u8 *)src;
    
    for (i = 0; i < size; i++) {
        ptr[i] = (u8)0x0;
    }
    
}

int memcmp(void *src1, void *src2, size_t len)
{
    u8 *cmp1 = (u8 *)src1;
    u8 *cmp2 = (u8 *)src2;
    
    ulong i;
    int result = 0;
    
    for (i = 0; i < len; i++) {
        if (cmp1[i] > cmp2[i]) {
            return 1;
        } else if (cmp1[i] < cmp2[i]) {
            return -1;
        }
    }
    
    return result;
}
