#include "common/include/data.h"
#include "hal/include/debug.h"
#include "hal/include/vector.h"


static enum int_vector_state int_vector_table[INT_VECTOR_COUNT];

static int_handler int_handler_list[INT_VECTOR_COUNT];


void init_int_vector()
{
    int i;
    
    // 0 - 63 Reserved by Intel
    for (i = 0; i < INT_VECTOR_ALLOC_START; i++) {
        int_vector_table[i] = int_vector_reserved;
    }
    
    // Allocatable
    for (i = INT_VECTOR_ALLOC_START; i <= INT_VECTOR_ALLOC_END; i++) {
        int_vector_table[i] = int_vector_free;
    }
}

int set_int_vector(int vector, int_handler hdlr)
{
    assert(int_vector_table[vector] == int_vector_free || int_vector_table[vector] == int_vector_reserved);
    
    int_vector_table[vector] = int_vector_allocated;
    
    if (NULL != hdlr) {
        int_handler_list[vector] = hdlr;
    }
    
    return 1;
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

int_handler get_int_handler(int vector)
{
    return int_handler_list[vector];
}
