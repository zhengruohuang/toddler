#ifndef __KERNEL_INCLUDE_PROC__
#define __KERNEL_INCLUDE_PROC__


#include "common/include/data.h"
#include "common/include/task.h"
#include "kernel/include/sync.h"


/*
 * Scheduling
 */
enum sched_state {
    sched_enter,
    sched_ready,
    sched_run,
    sched_idle,
    sched_stall,
    sched_exit,
};

struct sched {
    // Sched list
    struct sched *prev;
    struct sched *next;
    
    // Sched control info
    ulong sched_id;
    enum sched_state state;
    
    // Priority
    int base_priority;
    int priority;
    int is_idle;
    
    // Containing proc and thread
    ulong proc_id;
    struct process *proc;
    ulong thread_id;
    struct thread *thread;
    
    // Affinity
    int pin_cpu_id;
};

struct sched_list {
    ulong count;
    struct sched *next;
    struct sched *prev;
    
    spinlock_t lock;
};


/*
 * Thread
 */
enum thread_state {
    thread_enter,
    thread_normal,
    thread_stall,
    thread_wait,
    thread_exit,
    thread_clean,
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
    struct context context;
    
    // CPU affinity
    int pin_cpu_id;
    
    // Scheduling
    ulong sched_id;
    struct sched *sched;
};

struct thread_list {
    ulong count;
    struct thread *next;
    struct thread *prev;
    
    spinlock_t lock;
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

struct dynamic_area {
    ulong cur_top;
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
    
    // Dynamic area map
    struct dynamic_area dynamic;
    
    // Thread list
    struct {
        struct thread_list present;
        struct thread_list absent;
    } threads;
    
    // Scheduling
    uint priority;
};

struct process_list {
    ulong count;
    struct process  *next;
};


/*
 * Dynamic area
 */
extern void create_dalloc(struct process *p);
extern ulong dalloc(struct process *p, ulong size);
extern void dfree(struct process *p, ulong base);


/*
 * Process
 */
extern struct process *kernel_proc;;

extern void init_process();
extern struct process *create_process(
    ulong parent_id, char *name, char *url,
    enum process_type type, int priority
);
extern int load_image(struct process *p, char *url);


/*
 * Thread
 */
extern void init_thread();

extern void create_thread_lists(struct process *p);
extern struct thread *create_thread(
    struct process *p, ulong entry_point, ulong param,
    int pin_cpu_id,
    ulong stack_size, ulong tls_size
);
extern void destroy_absent_threads(struct process *p);

extern void run_thread(struct thread *t);
extern void idle_thread(struct thread *t);
extern void wait_thread(struct thread *t);
extern void terminate_thread(struct thread *t);
extern void clean_thread(struct thread *t);

extern void kernel_idle_thread(ulong param);
extern void kernel_demo_thread(ulong param);
extern void kernel_tclean_thread(ulong param);


/*
 * Scheduling
 */
extern void init_sched();
extern struct sched *get_sched(ulong sched_id);

extern struct sched *enter_sched(struct thread *t);
extern void ready_sched(struct sched *s);
extern void idle_sched(struct sched *s);
extern void exit_sched(struct sched *s);
extern void clean_sched(struct sched *s);

extern void desched(ulong sched_id, struct context *context);
extern void sched();
extern void resched(ulong sched_id, struct context *context);


#endif
