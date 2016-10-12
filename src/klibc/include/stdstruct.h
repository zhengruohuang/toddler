#ifndef __KLIBC_INCLUDE_STDSTRUCT__
#define __KLIBC_INCLUDE_STDSTRUCT__


#include "common/include/data.h"
#include "klibc/include/kthread.h"


/*
 * Doubly linked list
 */
typedef struct dlist_node {
    struct dlist_node *prev;
    struct dlist_node *next;
    
    void *node;
} dlist_node_t;

typedef struct dlist {
    unsigned int count;
    struct dlist_node *next;
    struct dlist_node *prev;
    
    kthread_mutex_t lock;
} dlist_t;

extern void dlist_create(dlist_t *l);
extern dlist_t *dlist_new();
extern void dlist_push_back(dlist_t *l, void *n);
extern void dlist_remove(dlist_t *l, dlist_node_t *s);
extern void *dlist_pop_front(dlist_t *l);


/*
 * Hash table
 */
typedef unsigned int (*hash_func_t)(void *key, unsigned int size);
typedef int (*hash_cmp_t)(void *cmp_key, void *node_key);

typedef struct hash_node {
    struct hash_node *next;
    void *key;
    void *node;
} hash_node_t;

typedef struct hash_bucket {
    unsigned int node_count;
    hash_node_t *head;
} hash_bucket_t;

typedef struct hash {
    unsigned int bucket_count;
    unsigned int node_count;
    hash_bucket_t *buckets;
    
    hash_func_t hash_func;
    hash_cmp_t hash_cmp;
    
    kthread_mutex_t lock;
} hash_t;

extern void hash_create(hash_t *l, unsigned int bucket_count, hash_func_t hash_func, hash_cmp_t hash_cmp);
extern hash_t *hash_new(unsigned int bucket_count, hash_func_t hash_func, hash_cmp_t hash_cmp);
extern int hash_contains(hash_t *l, void *key);
extern void *hash_obtain(hash_t *l, void *key);
extern void *hash_obtain_at(hash_t *l, unsigned long index);
extern void hash_release(hash_t *l, void *key, void *value);
extern int hash_insert(hash_t *l, void *key, void *value);
extern int hash_remove(hash_t *l, void *key);


#endif
