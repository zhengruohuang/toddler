#include "common/include/data.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/kthread.h"
#include "klibc/include/stdstruct.h"


static int dlist_salloc_id = -1;
static int dlist_node_salloc_id = -1;


static void init_dlist()
{
    dlist_salloc_id = salloc_create(sizeof(dlist_t), 0, NULL, NULL);
    dlist_node_salloc_id = salloc_create(sizeof(dlist_node_t), 0, NULL, NULL);
}

void dlist_create(dlist_t *l)
{
    if (dlist_node_salloc_id == -1) {
        init_dlist();
    }
    
    l->count = 0;
    l->next = NULL;
    l->prev = NULL;
    
    kthread_mutex_init(&l->lock);
}

dlist_t *dlist_new()
{
    dlist_t *l = NULL;
    
    if (dlist_salloc_id == -1) {
        init_dlist();
    }
    
    l = salloc(dlist_salloc_id);
    dlist_create(l);
    
    return l;
}

void dlist_push_back(dlist_t *l, void *n)
{
    // Allocate a dlist node
    dlist_node_t *s = (dlist_node_t *)salloc(dlist_node_salloc_id);
    //assert(s);
    s->node = n;
    
    kthread_mutex_lock(&l->lock);
    
    // Push back
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
    
    kthread_mutex_unlock(&l->lock);
}

static void inline dlist_detach(dlist_t *l, dlist_node_t *s)
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

void dlist_remove(dlist_t *l, dlist_node_t *s)
{
    kthread_mutex_lock(&l->lock);
    dlist_detach(l, s);
    kthread_mutex_unlock(&l->lock);
    
    sfree(s);
}

void *dlist_pop_front(dlist_t *l)
{
    dlist_node_t *s = NULL;
    void *n = NULL;
    
    kthread_mutex_lock(&l->lock);
    
    if (l->count) {
        //assert(l->next);
        
        s = l->next;
        dlist_detach(l, s);
    }
    
    kthread_mutex_unlock(&l->lock);
    
    n = s->node;
    sfree(s);
    
    return n;
}
