#include "common/include/data.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/kthread.h"
#include "klibc/include/assert.h"
#include "klibc/include/stdstruct.h"


static int hash_salloc_id;
static int hash_node_salloc_id;


static void init_hash()
{
    hash_salloc_id = salloc_create(sizeof(hash_t), 0, NULL, NULL);
    hash_node_salloc_id = salloc_create(sizeof(hash_node_t), 0, NULL, NULL);
}

static unsigned int default_hash_func(void *key, unsigned int size)
{
    unsigned int k = (unsigned int)(unsigned long)key;
    return k % size;
}

static int default_hash_cmp(void *cmp_key, void *node_key)
{
    unsigned long cmp = (unsigned long)cmp_key;
    unsigned long node = (unsigned long)node_key;
    
    if (cmp > node) {
        return 1;
    } else if (cmp == node) {
        return 0;
    } else {
        return -1;
    }
}

void hash_create(hash_t *l, unsigned int bucket_count, hash_func_t hash_func, hash_cmp_t hash_cmp)
{
    int i;
    
    if (!hash_node_salloc_id) {
        init_hash();
    }
    
    if (!l) {
        return;
    }
    
    l->bucket_count = bucket_count ? bucket_count : 16;
    l->node_count = 0;
    
    l->buckets = (hash_bucket_t *)malloc(sizeof(hash_bucket_t) * l->bucket_count);
    for (i = 0; i < l->bucket_count; i++) {
        l->buckets[i].node_count = 0;
        l->buckets[i].head = NULL;
    }
    
    l->hash_func = hash_func ? hash_func : default_hash_func;
    l->hash_cmp = hash_cmp ? hash_cmp : default_hash_cmp;
    
    kthread_mutex_init(&l->lock);
}

hash_t *hash_new(unsigned int bucket_count, hash_func_t hash_func, hash_cmp_t hash_cmp)
{
    hash_t *h = NULL;
    
    if (!hash_salloc_id) {
        init_hash();
    }
    
    h = salloc(hash_salloc_id);
    hash_create(h, bucket_count, hash_func, hash_cmp);
    
    return h;
}

int hash_contains(hash_t *l, void *key)
{
    int cmp = 0;
    int found = 0;
    hash_node_t *s = NULL;
    
    // Get the hash and bucket
    unsigned long hash = l->hash_func(key, l->bucket_count);
    hash_bucket_t *bucket = &l->buckets[hash];
    
    // Lock the table
    kthread_mutex_lock(&l->lock);
    
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
    kthread_mutex_unlock(&l->lock);
    
    return found;
}

void *hash_obtain(hash_t *l, void *key)
{
    int cmp = 0;
    int found = 0;
    hash_node_t *s = NULL;
    
    // Get the hash and bucket
    unsigned long hash = l->hash_func(key, l->bucket_count);
    hash_bucket_t *bucket = &l->buckets[hash];
    
    // Lock the table
    kthread_mutex_lock(&l->lock);
    
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
        kthread_mutex_unlock(&l->lock);
        return NULL;
    }
    
    return s->node;
}

void *hash_obtain_at(hash_t *l, unsigned long index)
{
    unsigned long count = 0;
    unsigned int i;
    hash_node_t *s = NULL;
    hash_bucket_t *bucket = NULL;
    
    // Lock the table
    kthread_mutex_lock(&l->lock);
    
    for (i = 0; i < l->bucket_count; i++) {
        bucket = &l->buckets[i];
        count += bucket->node_count;
        if (count > index) {
            count -= bucket->node_count;
            count = index - count;
            break;
        }
    }
    
    s = bucket->head;
    assert(s);
    for (i = 0; i < count; i++) {
        s = s->next;
        assert(s);
    }
    
    return s->node;
}

void hash_release(hash_t *l, void *key, void *n)
{
    //assert(n);
    if (!n) {
        return;
    }
    
    // Unlock
    kthread_mutex_unlock(&l->lock);
}

int hash_insert(hash_t *l, void *key, void *n)
{
    hash_node_t *cur = NULL;
    hash_node_t *prev = NULL;
    
    if (hash_contains(l, key)) {
        return -1;
    }
    
    // Allocate a node
    hash_node_t *s = (hash_node_t *)salloc(hash_node_salloc_id);
    if (!s) {
        return -2;
    }
    s->key = key;
    s->node = n;
    
    // Get the hash and bucket
    unsigned long hash = l->hash_func(key, l->bucket_count);
    hash_bucket_t *bucket = &l->buckets[hash];
    
    // Lock the table
    kthread_mutex_lock(&l->lock);
    
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
    kthread_mutex_unlock(&l->lock);
    
    return 0;
}

int hash_remove(hash_t *l, void *key)
{
    return 0;
}
