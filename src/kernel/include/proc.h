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
    thread_run,
    thread_stall,
    thread_wait,
    thread_exit,
};


struct thread_memory {
    ulong thread_block_base;
    
    ulong stack_top_offset;
    ulong stack_limit_offset;
    ulong tls_start_offset;
    
    ulong msg_recv_offset;
    ulong msg_send_offset;
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
    
    // CPU affinity
    int pin_cpu_id;
    
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
    
    // Process ID, -1 = No parent
    ulong proc_id;
    ulong parent_id;
    
    // Name and URL
    char *name;
    char *url;
    
    // Type and state
    enum process_type type;
    enum process_state state;
    int user_mode;
    
    // Page table
    ulong page_dir_pfn;
    
    // Virtual memory
    struct process_memory memory;
    
    // Thread list
    struct thread_list threads;
    
    // Scheduling
    uint priority;
};

struct process_list {
    ulong count;
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
    void *msg_recv;
    void *msg_send;
};



/*
 * Process
 */
extern struct process *kernel_proc;;

extern void init_process();


/*
 * Thread
 */
extern void init_thread();
extern void kernel_dummy_thread(ulong param);


#endif
