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
    spin_lock(&l->lock);
    
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
    
    spin_unlock(&l->lock);
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
    spin_lock(&l->lock);
    
    do_remove(l, s);
    
    spin_unlock(&l->lock);
}

static struct thread *pop_front(struct thread_list *l)
{
    if (!l->count) {
        return NULL;
    }
    
    struct thread *s = NULL;
    
    spin_lock(&l->lock);
    
    if (l->count) {
        assert(l->next);
        
        s = l->next;
        do_remove(l, s);
    }
    
    spin_unlock(&l->lock);
    
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
    ulong stack_size, ulong tls_size
)
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
    
    // Thread memory
    if (p->type == process_kernel) {
        t->memory.thread_block_base = PFN_TO_ADDR(palloc(4));
        
        t->memory.msg_send_offset = 0;
        t->memory.msg_recv_offset = PAGE_SIZE;
        
        t->memory.tls_start_offset = PAGE_SIZE * 2;
        
        t->memory.stack_limit_offset = PAGE_SIZE * 3;
        t->memory.stack_top_offset = PAGE_SIZE * 4;
    } else {
        // Round up stack size and tls size
        if (stack_size % PAGE_SIZE) {
            stack_size /= PAGE_SIZE;
            stack_size++;
            stack_size *= PAGE_SIZE;
        }
        
        if (tls_size % PAGE_SIZE) {
            tls_size /= PAGE_SIZE;
            tls_size++;
            tls_size *= PAGE_SIZE;
        }
        
        // Allocate a dynamic block
        ulong block_size = stack_size + tls_size + PAGE_SIZE * 2;
        t->memory.thread_block_base = dalloc(p, block_size);
        
        // Set up memory layout
        t->memory.msg_send_offset = 0;
        t->memory.msg_recv_offset = PAGE_SIZE;
        
        t->memory.tls_start_offset = PAGE_SIZE * 2;
        
        t->memory.stack_top_offset = block_size;
        t->memory.stack_limit_offset = block_size - stack_size;
        
        // Allocate memory and map it
        ulong paddr = PFN_TO_ADDR(palloc(1));
        assert(paddr);
        int succeed = hal->map_user(
            p->page_dir_pfn,
            t->memory.thread_block_base + t->memory.msg_send_offset,
            paddr, PAGE_SIZE, 0, 1, 1, 0
        );
        assert(succeed);
        
        paddr = PFN_TO_ADDR(palloc(1));
        assert(paddr);
        succeed = hal->map_user(
            p->page_dir_pfn,
            t->memory.thread_block_base + t->memory.msg_recv_offset,
            paddr, PAGE_SIZE, 0, 1, 1, 0
        );
        assert(succeed);
        
        paddr = PFN_TO_ADDR(palloc(tls_size / PAGE_SIZE));
        assert(paddr);
        succeed = hal->map_user(
            p->page_dir_pfn,
            t->memory.thread_block_base + t->memory.tls_start_offset,
            paddr, tls_size, 0, 1, 1, 0
        );
        assert(succeed);
        
        paddr = PFN_TO_ADDR(palloc(stack_size / PAGE_SIZE));
        assert(paddr);
        succeed = hal->map_user(
            p->page_dir_pfn,
            t->memory.thread_block_base + t->memory.stack_limit_offset,
            paddr, stack_size, 0, 1, 1, 0
        );
        assert(succeed);
    }
    
    // Prepare the param. Note that for user thread, we have to use indirect map, therefore we can't do a simple mem write
    if (p->type == process_kernel) {
        ulong *param_ptr = (ulong *)(t->memory.thread_block_base + t->memory.stack_top_offset - sizeof(ulong));
        *param_ptr = param;
    } else {
    }
    
    // Context
    hal->init_context(&t->context, entry_point,
                      t->memory.thread_block_base + t->memory.stack_top_offset - sizeof(ulong) * 2,
                      p->user_mode);
    
    // Scheduling
    t->sched = enter_sched(t);
    
    // Insert the thread into the thread list
    push_back(&p->threads.present, t);
    
    // Done
    return t;
}


/*
 * Thread destruction
 */
static void destroy_thread(struct process *p, struct thread *t)
{
    // Scheduling
    clean_sched(t->sched);
    
    // Dynamic area
    if (p->type == process_kernel) {
        pfree(ADDR_TO_PFN(t->memory.thread_block_base));
    } else {
        ulong vaddr = 0;
        ulong paddr = 0;
        
        // Msg send
        vaddr = t->memory.thread_block_base + t->memory.msg_send_offset;
        paddr = hal->get_paddr(p->page_dir_pfn, vaddr);
        pfree(paddr);
        
        // Msg send
        vaddr = t->memory.thread_block_base + t->memory.msg_recv_offset;
        paddr = hal->get_paddr(p->page_dir_pfn, vaddr);
        pfree(paddr);
        
        // TLS
        vaddr = t->memory.thread_block_base + t->memory.tls_start_offset;
        paddr = hal->get_paddr(p->page_dir_pfn, vaddr);
        pfree(paddr);
        
        // Stack
        vaddr = t->memory.thread_block_base + t->memory.stack_limit_offset;
        paddr = hal->get_paddr(p->page_dir_pfn, vaddr);
        pfree(paddr);
        
        // FIXME: also need to remove mapping
    }
    
    // Free thread struct
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
    t->state = thread_normal;
    ready_sched(t->sched);
}

void idle_thread(struct thread *t)
{
    t->state = thread_normal;
    idle_sched(t->sched);
}

void wait_thread(struct thread *t)
{
    t->state = thread_wait;
}

void terminate_thread(struct thread *t)
{
    t->state = thread_exit;
}

void clean_thread(struct thread *t)
{
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
        
        kprintf("\tKernel idle thread for CPU #%d created, thread ID: %p, thraed block base: %p\n", i, t->thread_id, t->memory.thread_block_base);
    }
    
    // Create kernel demo threads
    for (i = 0; i < 8; i++) {
        ulong param = i;
        struct thread *t = create_thread(kernel_proc, (ulong)&kernel_demo_thread, param, -1, 0, 0);
        run_thread(t);
        
        kprintf("\tKernel demo thread created, thread ID: %p, thraed block base: %p\n", t->thread_id, t->memory.thread_block_base);
    }
}
