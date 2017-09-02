#ifndef __HAL_INCLUDE_INT__
#define __HAL_INCLUDE_INT__


#include "common/include/data.h"
#include "common/include/context.h"
#include "common/include/kdisp.h"


/*
 * Interrupt handler
 */
struct int_context {
    ulong vector;
    ulong error_code;
    
    struct context *context;
};

typedef int (*int_handler)(struct int_context *intc, struct kernel_dispatch_info *kdi);


/*
 * Interrupt vectors
 */
#define INT_VECTOR_ALLOC_START      64
#define INT_VECTOR_ALLOC_END        255
#define INT_VECTOR_COUNT            (INT_VECTOR_ALLOC_END + 1)

enum int_vector_state {
    int_vector_unknown,
    int_vector_reserved,
    int_vector_free,
    int_vector_allocated,
    int_vector_other
};

extern void init_int_vector();
extern int set_int_vector(int vector, int_handler hdlr);
extern int alloc_int_vector(int_handler hdlr);
extern void free_int_vector(int vector);
extern int_handler get_int_handler(int vector);


#endif
