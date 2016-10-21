#include "common/include/data.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"


size_t strlen(const char *s)
{
    size_t len = 0;
    
    while (*s++) {
        len++;
    }
    
    return len;
}

int strcmp(const char *s1, const char *s2)
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

void strcpy(char *dest, const char *src)
{
    do {
        *dest++ = *src;
    } while (*src++);
    
    *dest = '\0';
}

char *strdup(const char *str)
{
    size_t len = strlen(str);
    char *buf = malloc(sizeof(char) * (len + 1));
    strcpy(buf, str);
    return buf;
}
