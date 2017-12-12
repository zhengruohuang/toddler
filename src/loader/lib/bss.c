#include "common/include/data.h"
#include "loader/include/lib.h"


extern int __bss_start;
extern int __bss_end;


void init_bss()
{
    int *start = (int *)ALIGN_UP((ulong)&__bss_start, sizeof(int));
    int *cur;
    
    for (cur = start; cur < &__bss_end; cur++) {
        *cur = 0;
    }
}
