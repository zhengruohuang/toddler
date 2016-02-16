#include "common/include/data.h"


void asmlinkage _start()
{
    int start_param = 0;
    
    switch (start_param) {
    case 0:
        /* Save the parameters */
        //hal_print_current_row_number = hal_start_param->line_number;
        //hal_print_current_column_number = hal_start_param->column_number;
        //hal_global_memory_pool_start = hal_start_param->hal_vaddr_end + 4096;
        
        /* Switch stack to HAL's, and thus this function is unable to return */
        __asm__ __volatile__
        (
            "xchgw  %%bx, %%bx;"
            "movl   %%eax, %%esp;"
            :
            : "a" (0xFFC02000)
        );
        
        break;
        
    /* Start AP */
    case 1:
        break;
        
    /* Return from BIOS Invoker */
    case 2:
        break;
        
    /* Stop */
    default:
        /* Switch stack to HAL's, and thus this function is unable to return */
        do {
            __asm__ __volatile__
            (
                "hlt;"
                :
                :
            );
        } while (1);
        
        break;
    }
    
    /* Call the entry of HAL */
    //hal_entry();
    
    do {
        __asm__ __volatile__
        (
            "hlt;"
            :
            :
        );
    } while (1);
}
