#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/sync.h"
#include "kernel/include/ds.h"


static int hashtable_salloc_id;
static int hash_node_salloc_id;


static ulong default_hash_func(ulong key, ulong size)
{
    return key % size;
}

static int default_hash_cmp(ulong cmp, ulong node)
{
    if (cmp > node) {
        return 1;
    } else if (cmp == node) {
        return 0;
    } else {
        return -1;
    }
}


void init_hashtable()
{
    hashtable_salloc_id = salloc_create(sizeof(hashtable_t), 0, 0, NULL, NULL);
    hash_node_salloc_id = salloc_create(sizeof(hashtable_node_t), 0, 0, NULL, NULL);
    kprintf("\tHashtable salloc ID: %d, node salloc ID: %d\n", hashtable_salloc_id, hash_node_salloc_id);
}

void hashtable_create(hashtable_t *l, ulong bucket_count, hashtable_func_t hash_func, hashtable_cmp_t hash_cmp)
{
    int i;
    
    l->bucket_count = bucket_count ? bucket_count : 16;
    l->node_count = 0;
    
    l->buckets = (hashtable_bucket_t *)malloc(sizeof(hashtable_bucket_t) * l->bucket_count);
    for (i = 0; i < l->bucket_count; i++) {
        l->buckets[i].node_count = 0;
        l->buckets[i].head = NULL;
    }
    
    l->hash_func = hash_func ? hash_func : default_hash_func;
    l->hash_cmp = hash_cmp ? hash_cmp : default_hash_cmp;
    
    spin_init(&l->lock);
}

hashtable_t *hashtable_new(ulong bucket_count, hashtable_func_t hash_func, hashtable_cmp_t hash_cmp)
{
    hashtable_t *h = salloc(hashtable_salloc_id);
    hashtable_create(h, bucket_count, hash_func, hash_cmp);
    
    return h;
}

int hashtable_contains(hashtable_t *l, ulong key)
{
    int cmp = 0;
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
        cmp = l->hash_cmp(key, s->key);
        if (0 == cmp) {
            found = 1;
            break;
        } else if (-1 == cmp) {
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
    int cmp = 0;
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
        cmp = l->hash_cmp(key, s->key);
        if (0 == cmp) {
            found = 1;
            break;
        } else if (-1 == cmp) {
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
    hashtable_node_t *cur = NULL;
    hashtable_node_t *prev = NULL;
    
    if (hashtable_contains(l, key)) {
        return 0;
    }
    
    // Allocate a node
    hashtable_node_t *s = (hashtable_node_t *)salloc(hash_node_salloc_id);
    assert(s);
    s->key = key;
    s->node = n;
    
    // Get the hash and bucket
    unsigned long hash = l->hash_func(key, l->bucket_count);
    hashtable_bucket_t *bucket = &l->buckets[hash];
    
    // Lock the table
    spin_lock_int(&l->lock);
    
    // Insert into the proper position
    cur = bucket->head;
    
    while (cur) {
        if (-1 == l->hash_cmp(key, cur->key)) {
            break;
        }
        
        prev = cur;
        cur = cur->next;
    }
    
    if (prev) {
        s->next = prev->next;
        prev->next = s;
    } else {
        s->next = bucket->head;
        bucket->head = s;
    }
    
    bucket->node_count++;
    l->node_count++;
    
    // Unlock
    spin_unlock_int(&l->lock);
    
    return 1;
}

int hashtable_remove(hashtable_t *l, ulong key)
{
    int cmp = 0;
    int found = 0;
    hashtable_node_t *s = NULL, *prev = NULL;
    
    // Get the hash and bucket
    ulong hash = l->hash_func(key, l->bucket_count);
    hashtable_bucket_t *bucket = &l->buckets[hash];
    
    // Lock the table
    spin_lock_int(&l->lock);
    
    // Find the node
    s = bucket->head;
    while (s) {
        cmp = l->hash_cmp(key, s->key);
        if (0 == cmp) {
            found = 1;
            break;
        } else if (-1 == cmp) {
            break;
        }
        
        prev = s;
        s = s->next;
    }
    
    if (found) {
        if (prev) {
            prev->next = s->next;
        } else {
            bucket->head = s->next;
        }
    }
    
    // Unlock
    spin_unlock_int(&l->lock);
    
    sfree(s);
    
    return found;
}
