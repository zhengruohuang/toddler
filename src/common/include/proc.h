#ifndef __COMMON_INCLUDE_PROC__
#define __COMMON_INCLUDE_PROC__


#include "common/include/data.h"


/*
 * Thread Control Block
 */
struct thread_control_block {
    struct thread_control_block *self;
    
    ulong proc_id;
    ulong thread_id;
    
    int cpu_id;
    
    void *tls;
    void *msg;
} packedstruct;


#endif
