#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/sync.h"
#include "kernel/include/ds.h"


static int hash_node_salloc_id;


void init_hashtable()
{
    hash_node_salloc_id = salloc_create(sizeof(hashtable_node_t), 0, 0, NULL, NULL);
    kprintf("\tHashtable node salloc ID: %d\n", hash_node_salloc_id);
}

static asmlinkage ulong default_hash_func(ulong key, ulong size)
{
    return key % size;
}

void hashtable_create(hashtable_t *l, ulong bucket_count, hashtable_func_t hash_func)
{
    int i;
    
    l->bucket_count = bucket_count ? bucket_count : 16;
    l->node_count = 0;
    l->buckets = (hashtable_bucket_t *)malloc(sizeof(hashtable_bucket_t) * l->bucket_count);
    l->hash_func = hash_func ? hash_func : default_hash_func;
    
    for (i = 0; i < l->bucket_count; i++) {
        l->buckets[i].node_count = 0;
        l->buckets[i].head = NULL;
    }
    
    spin_init(&l->lock);
}

int hashtable_contains(hashtable_t *l, ulong key)
{
    int found = 0;
    hashtable_node_t *s = NULL;
    
    // Get the hash and bucket
    ulong hash = l->hash_func(key, l->bucket_count);
    hashtable_bucket_t *bucket = &l->buckets[hash];
    
    // Lock the table
    spin_lock_int(&l->lock);
    
    // Find the node
    s = bucket->head;
    while (s) {
        if (s->key == key) {
            found = 1;
            break;
        }
        s = s->next;
    }
    
    // Unlock
    spin_unlock_int(&l->lock);
    
    return found;
}

void *hashtable_obtain(hashtable_t *l, ulong key)
{
    int found = 0;
    hashtable_node_t *s = NULL;
    
    // Get the hash and bucket
    ulong hash = l->hash_func(key, l->bucket_count);
    hashtable_bucket_t *bucket = &l->buckets[hash];
    
    // Lock the table
    spin_lock_int(&l->lock);
    
    // Find the node
    s = bucket->head;
    while (s) {
        if (s->key == key) {
            kprintf("Searching key: %x, compare: %x\n", s->key, key);
            found = 1;
            break;
        }
        s = s->next;
    }
    
    if (!found) {
        // Unlock
        spin_unlock_int(&l->lock);
        return NULL;
    }
    
    return s->node;
}

void hashtable_release(hashtable_t *l, ulong key, void *n)
{
    assert(n);
    
    // Unlock
    spin_unlock_int(&l->lock);
}

int hashtable_insert(hashtable_t *l, ulong key, void *n)
{
    if (hashtable_contains(l, key)) {
        return 0;
    }
    
    // Allocate a node
    hashtable_node_t *s = (hashtable_node_t *)salloc(hash_node_salloc_id);
    assert(s);
    s->key = key;
    s->node = n;
    
    // Get the hash and bucket
    ulong hash = l->hash_func(key, l->bucket_count);
    hashtable_bucket_t *bucket = &l->buckets[hash];
    
    // Lock the table
    spin_lock_int(&l->lock);
    
    // Push
    s->next = bucket->head;
    bucket->head = s;
    
    bucket->node_count++;
    l->node_count++;
    
    // Unlock
    spin_unlock_int(&l->lock);
    
    return 1;
}

int hashtable_remove(hashtable_t *l, ulong key)
{
    return 0;
}
