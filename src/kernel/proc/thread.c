/*
 * Thread manager
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


static int thread_salloc_id;


/*
 * Thread ID
 */
static ulong gen_thread_id(struct thread *t)
{
    ulong id = (ulong)t;
    return id;
}

struct thread *gen_thread_by_thread_id(ulong thread_id)
{
    return (struct thread *)thread_id;
}


/*
 * List
 */
static void init_list(struct thread_list *l)
{
    l->count = 0;
    l->next = NULL;
    l->prev = NULL;
    
    spin_init(&l->lock);
}

static void push_back(struct thread_list *l, struct thread *s)
{
    spin_lock_int(&l->lock);
    
    s->next = NULL;
    s->prev = l->prev;
    
    if (l->prev) {
        l->prev->next = s;
    }
    l->prev = s;
    
    if (!l->next) {
        l->next = s;
    }
    
    l->count++;
    
    spin_unlock_int(&l->lock);
}

static void inline do_remove(struct thread_list *l, struct thread *s)
{
    if (s->prev) {
        s->prev->next = s->next;
    }
    
    if (s->next) {
        s->next->prev = s->prev;
    }
    
    if (l->prev == s) {
        l->prev = s->prev;
    }
    
    if (l->next == s) {
        l->next = s->next;
    }
    
    l->count--;
}

static void remove(struct thread_list *l, struct thread *s)
{
    spin_lock_int(&l->lock);
    
    do_remove(l, s);
    
    spin_unlock_int(&l->lock);
}

static struct thread *pop_front(struct thread_list *l)
{
    if (!l->count) {
        return NULL;
    }
    
    struct thread *s = NULL;
    
    spin_lock_int(&l->lock);
    
    if (l->count) {
        assert(l->next);
        
        s = l->next;
        do_remove(l, s);
    }
    
    spin_unlock_int(&l->lock);
    
    return s;
}


/*
 * Thread creation
 */
void create_thread_lists(struct process *p)
{
    init_list(&p->threads.present);
    init_list(&p->threads.absent);
}

struct thread *create_thread(
    struct process *p, ulong entry_point, ulong param,
    int pin_cpu_id,
    ulong stack_size, ulong tls_size)
{
    // Allocate a thread struct
    struct thread *t = (struct thread *)salloc(thread_salloc_id);
    assert(t);
    
    // Assign a thread id
    t->thread_id = gen_thread_id(t);
    
    // Setup the thread
    t->proc_id = p->proc_id;
    t->proc = p;
    t->state = thread_enter;
    
    // Round up stack size and tls size
    if (!stack_size) {
        stack_size = PAGE_SIZE;
    }
    
    if (stack_size % PAGE_SIZE) {
        stack_size /= PAGE_SIZE;
        stack_size++;
        stack_size *= PAGE_SIZE;
    }
    
    if (!tls_size) {
        tls_size = PAGE_SIZE;
    }
    
    if (tls_size % PAGE_SIZE) {
        tls_size /= PAGE_SIZE;
        tls_size++;
        tls_size *= PAGE_SIZE;
    }
    
    // Setup sizes
    t->memory.msg_send_size = PAGE_SIZE;
    t->memory.msg_recv_size = PAGE_SIZE;
    t->memory.tls_size = tls_size;
    t->memory.stack_size = stack_size;
    t->memory.block_size = t->memory.msg_send_size + t->memory.msg_recv_size + t->memory.tls_size + t->memory.stack_size;
    
    // Setup offsets
    t->memory.msg_send_offset = 0;
    t->memory.msg_recv_offset = t->memory.msg_send_offset + t->memory.msg_send_size;
    t->memory.tls_start_offset = t->memory.msg_recv_offset + t->memory.msg_recv_size;
    t->memory.stack_limit_offset = t->memory.tls_start_offset + t->memory.tls_size;
    t->memory.stack_top_offset = t->memory.stack_limit_offset + stack_size;
    
    // Allocate memory
    if (p->type == process_kernel) {
        t->memory.block_base = PFN_TO_ADDR(palloc(t->memory.block_size / PAGE_SIZE));
        
        t->memory.msg_send_paddr = t->memory.block_base + t->memory.msg_send_offset;
        t->memory.msg_recv_paddr = t->memory.block_base + t->memory.msg_recv_offset;
        t->memory.tls_start_paddr = t->memory.block_base + t->memory.tls_start_offset;
        t->memory.stack_top_paddr = t->memory.block_base + t->memory.stack_top_offset;
    } else {
        // Allocate a dynamic block
        t->memory.block_base = dalloc(p, t->memory.block_size);
        
        // Allocate memory and map it
        ulong paddr = PFN_TO_ADDR(palloc(t->memory.msg_send_size / PAGE_SIZE));
        assert(paddr);
        int succeed = hal->map_user(
            p->page_dir_pfn,
            t->memory.block_base + t->memory.msg_send_offset,
            paddr, t->memory.msg_send_size, 0, 1, 1, 0
        );
        assert(succeed);
        t->memory.msg_send_paddr = paddr;
        
        paddr = PFN_TO_ADDR(palloc(t->memory.msg_recv_size / PAGE_SIZE));
        assert(paddr);
        succeed = hal->map_user(
            p->page_dir_pfn,
            t->memory.block_base + t->memory.msg_recv_offset,
            paddr, t->memory.msg_recv_size, 0, 1, 1, 0
        );
        assert(succeed);
        t->memory.msg_recv_paddr = paddr;
        
        paddr = PFN_TO_ADDR(palloc(tls_size / PAGE_SIZE));
        assert(paddr);
        succeed = hal->map_user(
            p->page_dir_pfn,
            t->memory.block_base + t->memory.tls_start_offset,
            paddr, tls_size, 0, 1, 1, 0
        );
        assert(succeed);
        t->memory.tls_start_paddr = paddr;
        
        paddr = PFN_TO_ADDR(palloc(stack_size / PAGE_SIZE));
        assert(paddr);
        succeed = hal->map_user(
            p->page_dir_pfn,
            t->memory.block_base + t->memory.stack_limit_offset,
            paddr, stack_size, 0, 1, 1, 0
        );
        assert(succeed);
        t->memory.stack_top_paddr = paddr + stack_size;
    }
    
    // Prepare the param
    ulong *param_ptr = (ulong *)(t->memory.stack_top_paddr - sizeof(ulong));
    *param_ptr = param;
    
    // Context
    hal->init_context(&t->context, entry_point, param,
                      t->memory.block_base + t->memory.stack_top_offset - sizeof(ulong) * 2,
                      p->user_mode);
    
    // Scheduling
    t->sched = enter_sched(t);
    
    // Init lock
    spin_init(&t->lock);
    
    // Insert the thread into the thread list
    push_back(&p->threads.present, t);
    
    // Done
    return t;
}


/*
 * Thread control
 */
void set_thread_arg(struct thread *t, ulong arg)
{
    ulong *param_ptr = NULL;
    
    spin_lock_int(&t->lock);
    
    param_ptr = (ulong *)(t->memory.stack_top_paddr - sizeof(ulong));
    *param_ptr = arg;
    
    hal->set_context_param(&t->context, arg);
    
    spin_unlock_int(&t->lock);
}

void change_thread_control(struct thread *t, ulong entry_point, ulong param)
{
    ulong *param_ptr = NULL;
    struct process *p;
    
    spin_lock_int(&t->lock);
    
    // Prepare the param
    param_ptr = (ulong *)(t->memory.stack_top_paddr - sizeof(ulong));
    *param_ptr = param;
    
    // Context
    p = t->proc;
    assert(entry_point);
    hal->init_context(&t->context, entry_point, param,
                      t->memory.block_base + t->memory.stack_top_offset - sizeof(ulong) * 2,
                      p->user_mode);
    
    spin_unlock_int(&t->lock);
}


/*
 * Thread destruction
 */
static void destroy_thread(struct process *p, struct thread *t)
{
    //kprintf("[Thread] To destory absent thread, process: %s\n", p->name);
    
    spin_lock_int(&t->lock);
    
    // Scheduling
    clean_sched(t->sched);
    
    // Dynamic area
    if (p->type == process_kernel) {
        pfree(ADDR_TO_PFN(t->memory.block_base));
    } else {
        ulong vaddr = 0;
        ulong paddr = 0;
        
        // TLB shootdown first
        trigger_tlb_shootdown(t->memory.block_base, t->memory.block_size);
        
        // Msg send
        vaddr = t->memory.block_base + t->memory.msg_send_offset;
        paddr = t->memory.msg_send_paddr;
        hal->unmap_user(p->page_dir_pfn, vaddr, paddr, t->memory.msg_send_size);
        pfree(ADDR_TO_PFN(paddr));
        
        // Msg send
        vaddr = t->memory.block_base + t->memory.msg_recv_offset;
        paddr = t->memory.msg_recv_paddr;
        hal->unmap_user(p->page_dir_pfn, vaddr, paddr, t->memory.msg_recv_size);
        pfree(ADDR_TO_PFN(paddr));
        
        // TLS
        vaddr = t->memory.block_base + t->memory.tls_start_offset;
        paddr = t->memory.tls_start_paddr;
        hal->unmap_user(p->page_dir_pfn, vaddr, paddr, t->memory.tls_size);
        pfree(ADDR_TO_PFN(paddr));
        
        // Stack
        vaddr = t->memory.block_base + t->memory.stack_limit_offset;
        paddr = hal->get_paddr(p->page_dir_pfn, vaddr);
        hal->unmap_user(p->page_dir_pfn, vaddr, paddr, t->memory.stack_size);
        pfree(ADDR_TO_PFN(paddr));
        
        // Free dynamic area
        dfree(p, t->memory.block_base);
    }
    
    spin_unlock_int(&t->lock);
    
    sfree(t);
}

void destroy_absent_threads(struct process *p)
{
    struct thread *t = pop_front(&p->threads.absent);
    while (t) {
        destroy_thread(p, t);
        t = pop_front(&p->threads.absent);
    }
}


/*
 * Change thread state
 */
void run_thread(struct thread *t)
{
    spin_lock_int(&t->lock);
    
    assert(t->state == thread_enter || t->state == thread_wait || t->state == thread_stall || t->state == thread_sched);
    
    t->state = thread_normal;
    ready_sched(t->sched);
    
    spin_unlock_int(&t->lock);
}

void idle_thread(struct thread *t)
{
    spin_lock_int(&t->lock);
    
    assert(t->state == thread_enter || t->state == thread_sched);
    
    t->state = thread_normal;
    idle_sched(t->sched);
    
    spin_unlock_int(&t->lock);
}

int wait_thread(struct thread *t)
{
    spin_lock_int(&t->lock);
    
    assert(t->state == thread_sched);
    
    t->state = thread_wait;
    wait_sched(t->sched);
    
    spin_unlock_int(&t->lock);
    
    return 1;
}

void terminate_thread_self(struct thread *t)
{
    enum thread_state state;
    //kprintf("To terminate thread, process: %s\n", t->proc->name);
    
    spin_lock_int(&t->lock);
    
    state = t->state;
    assert(state == thread_sched || state == thread_normal || state == thread_wait || state == thread_stall);
    
    //kprintf("thread state: %d\n", t->state);
    
    t->state = thread_exit;
    if (state == thread_wait || state == thread_stall) {
        exit_sched(t->sched);
        clean_thread(t);
    }
    
    spin_unlock_int(&t->lock);
}

void terminate_thread(struct thread *t)
{
    terminate_thread_self(t);
}

void clean_thread(struct thread *t)
{
    assert(t->state == thread_exit);
    t->state = thread_clean;
    
    remove(&t->proc->threads.present, t);
    push_back(&t->proc->threads.absent, t);
}


/*
 * Initialization
 */
void init_thread()
{
    kprintf("Initializing thread manager\n");
    
    // Create salloc obj
    thread_salloc_id = salloc_create(sizeof(struct thread), 0, 0, NULL, NULL);
    kprintf("\tThread salloc ID: %d\n", thread_salloc_id);
    
    // Create idel kernel threads, one for each CPU
    int i;
    for (i = 0; i < hal->num_cpus; i++) {
        ulong param = i;
        struct thread *t = create_thread(kernel_proc, (ulong)&kernel_idle_thread, param, i, 0, 0);
        idle_thread(t);
        
        kprintf("\tKernel idle thread for CPU #%d created, thread ID: %p, thraed block base: %p\n", i, t->thread_id, t->memory.block_base);
    }
    
//     // Create kernel demo threads
//     for (i = 0; i < 2; i++) {
//         ulong param = i;
//         struct thread *t = create_thread(kernel_proc, (ulong)&kernel_demo_thread, param, -1, 0, 0);
//         run_thread(t);
//         
//         kprintf("\tKernel demo thread created, thread ID: %p, thraed block base: %p\n", t->thread_id, t->memory.block_base);
//     }
    
    // Create kernel thread cleaner
    struct thread *t = create_thread(kernel_proc, (ulong)&kernel_tclean_thread, (ulong)kernel_proc, -1, 0, 0);
    run_thread(t);
    kprintf("\tKernel cleaner thread created, thread ID: %p, thraed block base: %p\n", t->thread_id, t->memory.block_base);
}
