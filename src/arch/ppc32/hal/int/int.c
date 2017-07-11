#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"


extern int __int_entry_wrapper_begin;
extern int __int_entry_wrapper_end;


void init_int()
{
    ulong len = (ulong)&__int_entry_wrapper_end - (ulong)&__int_entry_wrapper_begin;
    
    kprintf("Initializing interrupt handlers\n");
    
    // Map the area
    fill_kernel_pht(0, len, 0, 1);
    
    // Copy the code
    memcpy((void *)0, &__int_entry_wrapper_begin, len);
    kprintf("\tHandler template copied @ %p -> %p, size: %p\n",
            &__int_entry_wrapper_begin, (void *)0, (void *)len);
}
