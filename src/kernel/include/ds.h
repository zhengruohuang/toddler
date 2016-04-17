#ifndef __KERNEL_INCLUDE_DATA_STRUCTURE__
#define __KERNEL_INCLUDE_DATA_STRUCTURE__


#include "common/include/data.h"
#include "kernel/include/sync.h"


/*
 * Doubly linked list
 */
typedef struct list_node {
    struct list_node *prev;
    struct list_node *next;
    
    void *node;
} list_node_t;

typedef struct list {
    ulong count;
    struct list_node *next;
    struct list_node *prev;
    
    spinlock_t lock;
} list_t;

//extern void init_list();
extern void list_create(list_t *l);
extern void list_push_back(list_t *l, void *n);
extern void list_remove(list_t *l, list_node_t *s);
extern void *list_pop_front(list_t *l);


/*
 * Hash table
 */
typedef asmlinkage ulong (*hashtable_func_t)(ulong key, ulong size);

typedef struct hashtable_node {
    struct hashtable_node *next;
    ulong key;
    void *node;
} hashtable_node_t;

typedef struct hashtable_bucket {
    ulong node_count;
    hashtable_node_t *head;
} hashtable_bucket_t;

typedef struct hashtable {
    ulong bucket_count;
    ulong node_count;
    hashtable_func_t hash_func;
    hashtable_bucket_t *buckets;
    
    spinlock_t lock;
} hashtable_t;

extern void init_hashtable();
extern void hashtable_create(hashtable_t *l, ulong bucket_count, hashtable_func_t hash_func);
extern int hashtable_contains(hashtable_t *l, ulong key);
extern void *hashtable_obtain(hashtable_t *l, ulong key);
extern int hashtable_insert(hashtable_t *l, ulong key, void *n);
extern int hashtable_remove(hashtable_t *l, ulong key);


#endif
