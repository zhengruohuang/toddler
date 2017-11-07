#include "common/include/data.h"


extern int __bss_start;
extern int __bss_end;


void init_bss()
{
    int *cur;
    
    for (cur = &__bss_start; cur < &__bss_end; cur++) {
        *cur = 0;
    }
}
