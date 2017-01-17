#ifndef __COMMON_INCLUDE_PROC__
#define __COMMON_INCLUDE_PROC__


#include "common/include/data.h"


/*
 * Process Control Block
 */
struct process_control_block {
    struct process_control_block *self;
    
    ulong proc_id;
} packedstruct;


/*
 * Thread Control Block
 */
struct thread_control_block {
    struct thread_control_block *self;
    
    ulong proc_id;
    ulong thread_id;
    
    ulong cpu_id;
    
    void *tls;
    void *msg_send;
    void *msg_recv;
} packedstruct;


#endif
