#include "common/include/data.h"
#include "klibc/include/string.h"


size_t strlen(char *s)
{
    size_t len = 0;
    
    while (*s++) {
        len++;
    }
    
    return len;
}

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

void strcpy(char *dest, char *src)
{
    do {
        *dest++ = *src;
    } while (*src++);
}
