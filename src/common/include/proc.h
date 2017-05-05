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


/*
 * KMap
 */
enum kmap_region {
    kmap_none,
    kmap_coreimg,
};


/*
 * Process monitor
 */
enum proc_monitor_type {
    pm_none,
    pm_create_before,
    pm_create_after,
    pm_terminate_before,
    pm_terminate_after,
    pm_type_count,
};


#endif
