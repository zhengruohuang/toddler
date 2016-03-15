#ifndef __KERNEL_INCLUDE_PROC__
#define __KERNEL_INCLUDE_PROC__


#include "common/include/data.h"


/*
 * Context
 */
struct context_info {

    
};

/*
 * Thread
 */
enum thread_state {
    thread_enter,
    thread_ready,
    thread_running,
    thread_sleep,
    thread_exit,
};


struct thread_memory {
};

struct thread {
    // Thread list
    struct thread *next;
    struct thread *prev;
    
    // Thread info
    ulong thread_id;
    enum thread_state state;
    struct thread_memory memory;
    
    // Containing process
    ulong proc_id;
    struct process *proc;
    
    // Context
    struct context_info context;
    
    // Scheduling
    ulong sched_id;
};

struct thread_list {
    ulong count;
    struct thread *next;
};


/*
 * Process
 */
enum process_type {
    // Native types
    process_kernel,
    process_driver,
    process_system,
    process_user,
    
    // Emulation - e.g. running Linux on top of Toddler
    process_emulate,
};

enum process_state {
    process_enter,
    process_normal,
    process_error,
    process_exit,
};

struct process_memory {
    // Entry point
    ulong entry_point;
    
    // Page table
    ulong page_table_pfn;
    
    // Memory layout
    ulong program_start;
    ulong program_end;
    
    ulong dynamic_top;
    ulong dynamic_bottom;
    
    ulong heap_start;
    ulong heap_end;
};

struct process {
    // Process list
    struct process *next;
    struct process *prev;
    
    // Process ID, -1 = Not Available
    ulong proc_id;
    ulong parent_id;
    
    // Name
    char *name;
    
    // Type and state
    enum process_type type;
    enum process_state state;
    
    // Virtual memory
    struct process_memory memory;
    
    // Thread list
    struct thread_list threads;
    
    // Scheduling
    uint priority;
};

struct process_list {
    struct process  *next;
};


/*
 * Thread Control Block
 */
struct thread_control_block {
    ulong proc_id;
    ulong thread_id;
    
    int cpu_id;
    
    void *tls;
};


#endif
