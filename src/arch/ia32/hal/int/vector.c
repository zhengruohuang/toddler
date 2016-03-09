#include "common/include/data.h"
#include "hal/include/lib.h"
#include "hal/include/int.h"


static enum int_vector_state int_vector_table[256];
static int syscall_vector[] = { 0x90 };


void init_int_vector()
{
    int i;
    
    // 0 - 31 Reserved by Intel
    for (i = 0; i < INT_VECTOR_ALLOC_START; i++) {
        int_vector_table[i] = int_vector_reserved;
    }
    
    // Allocatable
    for (i = INT_VECTOR_ALLOC_START; i <= INT_VECTOR_ALLOC_END; i++) {
        int_vector_table[i] = int_vector_free;
    }
    
    // Reserved by Syscalls
    for (i = 0; i < sizeof(syscall_vector) / sizeof(int); i++) {
        int_vector_table[syscall_vector[i]] = int_vector_reserved;
    }
}

int alloc_int_vector(int_handler hdlr)
{
    int i;
    
    for (i = INT_VECTOR_ALLOC_START; i <= INT_VECTOR_ALLOC_END; i++) {
        if (int_vector_free == int_vector_table[i]) {
            int_vector_table[i] = int_vector_allocated;
            
            if (NULL != hdlr) {
                int_handler_list[i] = hdlr;
            }
            
            return i;
        }
    }
    
    return 0;
}

void free_int_vector(int vector)
{
    assert(int_vector_allocated == int_vector_table[vector]);
    
    int_vector_table[vector] = int_vector_free;
    int_handler_list[vector] = NULL;
}
