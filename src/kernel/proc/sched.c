/*
 * Scheduler
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


static int sched_salloc_id;

static sched_list enter_queue;
static sched_list ready_queue;
static sched_list run_queue;
static sched_list exit_queue;


static ulong gen_sched_id(struct sched *s)
{
    ulong id = (ulong)s;
    return id;
}


/*
 * List
 */
static void insert(struct sched_list *l, struct sched *s)
{
    s->prev = NULL;
    s->next = l->next;
    l->next = s;
}

static void remove(struct sched_list *l, struct sched *s)
{
    if (s->prev) {
        s->prev->next = s->next;
    }
    
    if (s->next) {
        s->next->prev = s->prev;
    }
    
    if (l->next == s) {
        l->next = s->next;
    }
    l->count--;
}

static struct sched *pop(struct sched_list *l)
{
    struct sched *s = NULL;
    
    if (l->count) {
        assert(l->next);
        
        s = l->next;
        remove(l, s);
    }
    
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
    enter_queue.count = 0;
    enter_queue.next = NULL;
    
    ready_queue.count = 0;
    ready_queue.next = NULL;
    
    run_queue.count = 0;
    run_queue.next = NULL;
    
    exit_queue.count = 0;
    exit_queue.next = NULL;
    
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
    insert(&enter_queue, s);
}

void ready_sched(struct sched *s)
{
    // Before transitioning to ready, the thread must be in enter state
    assert(s->state == sched_enter);
    
    // Remove the entry from its current list
    remove(&enter_queue, s);
    
    // Insert s into the ready queue
    insert(&ready_queue, s);
}

void exit_sched(struct sched *s)
{
    // If the thread is running, we don't insert it into exit list immediately
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
        insert(&exit_queue, s);
    }
}

void clean_sched(struct sched *s)
{
    assert(s->state == sched_exit);
    
    remove(&exit_queue, s);
    sfree(s);
}


/*
 * The actual scheduler
 */
void sched()
{
    // We simply pop the first entry in the run queue
    struct sched *s = pop(&run_queue);
    
    if (s) {
        // Insert it into run queue
        insert(&run_queue, s);
        
        // Then tell HAL to do a context switch
    }
    
    // Nothing to do? switch to the dummy thread
    else {
    }
}
