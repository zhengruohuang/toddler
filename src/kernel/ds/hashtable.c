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

static ulong default_hash_func(ulong key, ulong size)
{
    return key % size;
}

void hashtable_create(hashtable_t *l, ulong bucket_count, void *hash_func)
{
    l->bucket_count = bucket_count;
    l->node_count = 0;
    l->buckets = (hashtable_bucket_t *)malloc(sizeof(hashtable_bucket_t *) * bucket_count);
    
    spin_init(&l->lock);
}

int hashtable_contains(hashtable_t *l, ulong key)
{
    int found = 0;
    hashtable_node_t *s = NULL;
    
    // Get the hash and bucket
    ulong hash = l->hash_func(key, l->bucket_count);
    hashtable_bucket_t *bucket = l->buckets[hash];
    
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
    
}

int hashtable_insert(hashtable_t *l, ulong key, void *n)
{
    if (hashtable_contains(l, key)) {
        return 0;
    }
    
    // Allocate a node
    hashtable_node_t *s = (hashtable_node_t *)salloc(hash_node_salloc_id);
    assert(s);
    s->node = n;
    
    // Get the hash and bucket
    ulong hash = l->hash_func(key, l->bucket_count);
    hashtable_bucket_t *bucket = l->buckets[hash];
    
    // Lock the table
    spin_lock_int(&l->lock);
    
    // Push
    s->next = bucket->head;
    bucket->head = s;
    
    bucket->node_count++;
    l->bucket_count++;
    
    // Unlock
    spin_unlock_int(&l->lock);
    
    return 1;
}

void hashtable_remove(hashtable_t *l, ulong key)
{

}
