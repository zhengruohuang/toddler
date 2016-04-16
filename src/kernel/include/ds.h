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


/*
 * Hash table
 */
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
    void *hash_func;
    hashtable_bucket_t *buckets;
    
    spinlock_t lock;
} hashtable_t;


#endif
