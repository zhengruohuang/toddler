#ifndef __KERNEL_INCLUDE_PROC__
#define __KERNEL_INCLUDE_PROC__


#include "common/include/data.h"
#include "common/include/task.h"
#include "kernel/include/ds.h"
#include "kernel/include/sync.h"
#include "common/include/syscall.h"


/*
 * IPC
 */
struct msg_node {
    struct {
        ulong mailbox_id;
        struct process *proc;
        struct thread *thread;
    } src;
    
    struct {
        ulong mailbox_id;
        struct process *proc;
        struct thread *thread;
    } dest;
    
    int sender_blocked;
    msg_t *msg;
};

struct msg_handler {
    ulong msg_num;
    ulong vaddr;
};


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
    struct sched *head;
    struct sched *tail;
    
    spinlock_t lock;
};


/*
 * Thread
 */
enum thread_state {
    // Thread just created
    thread_enter,
    
    // Thread is being sched
    thread_sched,
    
    // Thread running
    thread_normal,
    
    // Thread waiting for syscall or IO reponse
    thread_stall,
    thread_wait,
    
    // Thread waiting to be terminated
    thread_exit,
    
    // Thread terminated, waiting to be claned
    thread_clean,
};

struct thread_memory {
    // Virtual base
    ulong block_base;
    ulong block_size;
    
    // Stack
    ulong stack_top_offset;
    ulong stack_limit_offset;
    ulong stack_size;
    ulong stack_top_paddr;
    
    // TCB
    ulong tcb_start_offset;
    ulong tcb_size;
    ulong tcb_start_paddr;
    
    // TLS
    ulong tls_start_offset;
    ulong tls_size;
    ulong tls_start_paddr;
    
    // Msg recv
    ulong msg_recv_offset;
    ulong msg_recv_size;
    ulong msg_recv_paddr;
    
    // Msg send
    ulong msg_send_offset;
    ulong msg_send_size;
    ulong msg_send_paddr;
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
    
    // IPC
    struct msg_node *cur_msg;
    
    // Lock
    spinlock_t lock;
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

struct dynamic_block {
    struct dynamic_block *next;
    
    ulong base;
    ulong size;
};

struct dynamic_block_list {
    struct dynamic_block *head;
    int count;
};

struct dynamic_area {
    ulong cur_top;
    
    struct dynamic_block_list in_use;
    struct dynamic_block_list free;
};

struct process {
    // Process list
    struct process *next;
    struct process *prev;
    
    // Process ID, -1 = No parent
    ulong proc_id;
    ulong parent_id;
    
    // ASID
    ulong asid;
    
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
    
    // IPC
    ulong mailbox_id;
    list_t msgs;
    hashtable_t msg_handlers;
    
    // Lock
    spinlock_t lock;
};

struct process_list {
    ulong count;
    struct process *next;
    
    spinlock_t lock;
};


/*
 * Dynamic area
 */
extern void init_dalloc();
extern void create_dalloc(struct process *p);
extern ulong dalloc(struct process *p, ulong size);
extern void dfree(struct process *p, ulong base);


/*
 * Process
 */
extern struct process *kernel_proc;

extern void init_process();
extern struct process *create_process(
    ulong parent_id, char *name, char *url,
    enum process_type type, int priority
);
extern int load_image(struct process *p, char *url);


/*
 * Thread
 */
extern struct thread *gen_thread_by_thread_id(ulong thread_id);
extern void init_thread();

extern void create_thread_lists(struct process *p);
extern struct thread *create_thread(
    struct process *p, ulong entry_point, ulong param,
    int pin_cpu_id,
    ulong stack_size, ulong tls_size
);

extern void set_thread_arg(struct thread *t, ulong arg);
extern void change_thread_control(struct thread *t, ulong entry_point, ulong param);

extern void destroy_absent_threads(struct process *p);

extern void run_thread(struct thread *t);
extern void idle_thread(struct thread *t);
extern int wait_thread(struct thread *t);
extern void terminate_thread_self(struct thread *t);
extern void terminate_thread(struct thread *t);
extern void clean_thread(struct thread *t);

extern asmlinkage void kernel_idle_thread(ulong param);
extern asmlinkage void kernel_demo_thread(ulong param);
extern asmlinkage void kernel_tclean_thread(ulong param);


/*
 * Scheduling
 */
extern void init_sched();
extern struct sched *get_sched(ulong sched_id);

extern struct sched *enter_sched(struct thread *t);
extern void ready_sched(struct sched *s);
extern void idle_sched(struct sched *s);
extern void wait_sched(struct sched *s);
extern void exit_sched(struct sched *s);
extern void exit_sched(struct sched *s);
extern void clean_sched(struct sched *s);

extern int desched(ulong sched_id, struct context *context);
extern void resched(ulong sched_id);
extern void sched();


/*
 * TLB management
 */
extern void init_tlb_mgmt();
extern void trigger_tlb_shootdown(ulong asid, ulong addr, size_t size);
extern void service_tlb_shootdown();


/*
 * Heap management
 */
extern ulong set_heap_end(struct process *p, ulong heap_end);
extern ulong get_heap_end(struct process *p);
extern ulong grow_heap(struct process *p, ulong amount);
extern ulong shrink_heap(struct process *p, ulong amount);


/*
 * ASID management
 */
extern void init_asid();
extern void asid_release();
extern ulong asid_alloc();
extern ulong asid_recycle();


#endif
