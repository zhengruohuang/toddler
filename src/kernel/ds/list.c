#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/sync.h"
#include "kernel/include/ds.h"


static int list_node_salloc_id;


void init_list()
{
    list_node_salloc_id = salloc_create(sizeof(list_node_t), 0, 0, NULL, NULL);
    kprintf("\tDoubly linked list node salloc ID: %d\n", list_node_salloc_id);
}

void list_create(list_t *l)
{
    l->count = 0;
    l->next = NULL;
    l->prev = NULL;
    
    spin_init(&l->lock);
}

void list_push_back(list_t *l, void *n)
{
    // Allocate a list node
    list_node_t *s = (list_node_t *)salloc(list_node_salloc_id);
    assert(s);
    s->node = n;
    
    spin_lock_int(&l->lock);
    
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
    
    spin_unlock_int(&l->lock);
}

static void inline list_detach(list_t *l, list_node_t *s)
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

void list_remove(list_t *l, list_node_t *s)
{
    spin_lock_int(&l->lock);
    list_detach(l, s);
    spin_unlock_int(&l->lock);
    
    sfree(s);
}

void *list_pop_front(list_t *l)
{
    list_node_t *s = NULL;
    void *n = NULL;
    
    spin_lock_int(&l->lock);
    
    if (l->count) {
        assert(l->next);
        
        s = l->next;
        list_detach(l, s);
    }
    
    spin_unlock_int(&l->lock);
    
    n = s->node;
    sfree(s);
    
    return n;
}
