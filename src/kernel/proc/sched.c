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

static struct sched *get_sched(ulong sched_id)
{
    return (struct sched *)sched_id;
}


/*
 * List
 */
static void init_list(struct sched_list *l)
{
    l->count = 0;
    l->next = NULL;
    l->prev = NULL;
    
    spin_init(&l->lock);
}

static void push_back(struct sched_list *l, struct sched *s)
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

static void inline do_remove(struct sched_list *l, struct sched *s)
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

static void remove(struct sched_list *l, struct sched *s)
{
    spin_lock(&l->lock);
    
    do_remove(l, s);
    
    spin_unlock(&l->lock);
}

static struct sched *pop_front(struct sched_list *l)
{
    struct sched *s = NULL;
    
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
    assert(s->state == sched_enter);
    
    // Remove the entry from its current list
    remove(&enter_queue, s);
    
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
    
    // Then tell HAL to do a context switch
    hal->switch_context(s->sched_id, &s->thread->context, s->proc->page_dir_pfn, s->proc->user_mode, 0);
}
