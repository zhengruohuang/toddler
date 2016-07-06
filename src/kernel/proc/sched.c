/*
 * Scheduler
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"
#include "kernel/include/sync.h"
#include "kernel/include/proc.h"


static int sched_salloc_id;

static struct sched_list enter_queue;
static struct sched_list ready_queue;
static struct sched_list stall_queue;
static struct sched_list idle_queue;
static struct sched_list run_queue;
static struct sched_list exit_queue;


static ulong gen_sched_id(struct sched *s)
{
    ulong id = (ulong)s;
    return id;
}

struct sched *get_sched(ulong sched_id)
{
    return (struct sched *)sched_id;
}


/*
 * List
 */
static void init_list(struct sched_list *l)
{
    l->count = 0;
    l->head = NULL;
    l->tail = NULL;
    
    spin_init(&l->lock);
}

static void push_back(struct sched_list *l, struct sched *s)
{
    spin_lock_int(&l->lock);
    
    s->next = NULL;
    s->prev = NULL;
    
    if (l->count) {
        s->prev = l->tail;
        l->tail->next = s;
        l->tail = s;
    } else {
        l->tail = s;
        l->head = s;
    }
    
    l->count++;
    
//     if (l == &stall_queue) {
//         int printed = 0;
//         kprintf("List: %p, Push back: %p, Count: %d,", l, s, l->count);
//         struct sched *p = l->head;
//         while (p) {
//             kprintf(" %p", p);
//             p = p->next;
//             printed++;
//             
//             assert(printed <= l->count);
//         }
//         kprintf("\n");
//     }
    
    spin_unlock_int(&l->lock);
}

static void inline do_remove(struct sched_list *l, struct sched *s)
{
    if (l->count == 1) {
        assert(l->head == s);
        assert(l->tail == s);
        
        l->head = l->tail = NULL;
    } else if (l->head == s) {
        l->head = s->next;
        s->next->prev = NULL;
    } else if (l->tail == s) {
        l->tail = s->prev;
        s->prev->next = NULL;
    } else {
        s->prev->next = s->next;
        s->next->prev = s->prev;
    }
    
    l->count--;
    
//     if (l == &stall_queue) {
//         int printed = 0;
//         kprintf("List: %p, Do remove: %p, Count: %d,", l, s, l->count);
//         struct sched *p = l->head;
//         while (p) {
//             kprintf(" %p", p);
//             p = p->next;
//             printed++;
//             
//             assert(printed <= l->count);
//         }
//         kprintf("\n");
//     }
}

static void remove(struct sched_list *l, struct sched *s)
{
    spin_lock_int(&l->lock);
    
    do_remove(l, s);
    
    spin_unlock_int(&l->lock);
}

static struct sched *pop_front(struct sched_list *l)
{
    struct sched *s = NULL;
    
    spin_lock_int(&l->lock);
    
    if (l->count) {
        assert(l->head);
        
        s = l->head;
        do_remove(l, s);
    }
    
    spin_unlock_int(&l->lock);
    
    return s;
}


/*
 * Init
 */
void init_sched()
{
    kprintf("Initializing scheduler\n");
    
    // Create salloc obj
    sched_salloc_id = salloc_create(sizeof(struct sched), 0, 0, NULL, NULL);
    
    // Init the queues
    init_list(&enter_queue);
    init_list(&ready_queue);
    init_list(&idle_queue);
    init_list(&run_queue);
    init_list(&stall_queue);
    init_list(&exit_queue);
    
    // Done
    kprintf("\tScheduler salloc ID: %d\n", sched_salloc_id);
}


/*
 * Transitionings
 */
struct sched *enter_sched(struct thread *t)
{
    // Allocate a sched struct
    struct sched *s = (struct sched *)salloc(sched_salloc_id);
    assert(s);
    
    // Assign a sched id
    s->sched_id = gen_sched_id(s);
    
    // Setup the sched
    s->proc_id = t->proc_id;
    s->proc = t->proc;
    s->thread_id = t->thread_id;
    s->thread = t;
    s->state = sched_enter;
    
    // Insert sched into enter queue
    push_back(&enter_queue, s);
    
    return s;
}

void ready_sched(struct sched *s)
{
    // Before transitioning to ready, the thread must be in enter state
    assert(s->state == sched_enter || s->state == sched_stall);
    
    // Remove the entry from its current list
    if (s->state == sched_enter) {
        remove(&enter_queue, s);
    } else if (s->state == sched_stall) {
        remove(&stall_queue, s);
    }
    
    // Setup the sched struct
    s->is_idle = 0;
    
    // Insert s into the ready queue
    push_back(&ready_queue, s);
}

void idle_sched(struct sched *s)
{
    // Before transitioning to ready, the thread must be in enter state
    assert(s->state == sched_enter);
    
    // Remove the entry from its current list
    remove(&enter_queue, s);
    
    // Setup the sched struct
    s->is_idle = 1;
    
    // Insert s into the idle queue
    push_back(&idle_queue, s);
}

void exit_sched(struct sched *s)
{
    // If the thread is running, we don't push_back it into exit list immediately
    if (s->state == sched_run) {
        // To support lazy sched, we also need to notify the target processor
        // however this is necessary for now
        
        s->state = sched_exit;
        return;
    }
    
    else if (s->state == sched_exit) {
        return;
    }
    
    else {
        if (s->state == sched_ready) {
            remove(&ready_queue, s);
        } else if (s->state == sched_enter) {
            remove(&enter_queue, s);
        }
        
        s->state = sched_exit;
        push_back(&exit_queue, s);
    }
}

void clean_sched(struct sched *s)
{
    assert(s->state == sched_exit);
    
    remove(&exit_queue, s);
    sfree(s);
}


/*
 * Deschedule current thread
 */
void desched(ulong sched_id, struct context *context)
{
    if (!sched_id) {
        return;
    }
    
    //kprintf("Deschedule\n");
    
    // Get the sched struct
    struct sched *s = get_sched(sched_id);
    assert(s);
    assert(s->state == sched_run);
    remove(&run_queue, s);
    
    // Get thread struct
    struct thread *t = s->thread;
    
    // Save context
    assert(context);
    memcpy(context, &t->context, sizeof(struct context));
    
    // Setup thread state
    switch (t->state) {
    case thread_normal:
        s->state = sched_ready;
        push_back(&ready_queue, s);
        break;
    case thread_stall:
    case thread_wait:
        s->state = sched_stall;
        push_back(&stall_queue, s);
        break;
    case thread_exit:
        s->state = sched_exit;
        push_back(&exit_queue, s);
        clean_thread(t);
        break;
    default:
        panic("Unsupported thread state: %d", t->state);
        break;
    }
}

/*
 * The actual scheduler
 */
void sched()
{
    // We simply pop_front the first entry in the ready queue
    struct sched *s = pop_front(&ready_queue);
    
    // Nothing to run? Get an idle thread
    if (!s) {
        s = pop_front(&idle_queue);
        //kprintf("Idle\n");
    }
    
    assert(s);
    
    // Insert it into run queue
    s->state = sched_run;
    push_back(&run_queue, s);
    
//     kprintf("Process: %s, Context: eip: %p, esp: %p, cs: %p, ds: %p\n",
//            s->proc->name,
//            s->thread->context.eip,
//            s->thread->context.esp,
//            s->thread->context.cs,
//            s->thread->context.ds
//     );
    
    // Construct the TCB template
    struct thread_control_block tcb;
    ulong base = s->thread->memory.thread_block_base;
    tcb.msg_send = (void *)(base + s->thread->memory.msg_send_offset);
    tcb.msg_recv = (void *)(base + s->thread->memory.msg_recv_offset);
    tcb.tls = (void *)(base + s->thread->memory.tls_start_offset);
    tcb.proc_id = s->proc_id;
    tcb.thread_id = s->thread_id;
    
    // Then tell HAL to do a context switch
    hal->switch_context(s->sched_id, &s->thread->context, s->proc->page_dir_pfn, s->proc->user_mode, 0, &tcb);
}


/*
 * Deschedule + schedule
 */
void resched(ulong sched_id, struct context *context)
{
    desched(sched_id, context);
    sched();
}
