#include "common/include/data.h"
#include "common/include/atomic.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"


/*
 * Magic block
 */
struct salloc_magic_block {
    struct salloc_bucket *bucket;
    struct salloc_magic_block *next;
};


/*
 * Bucket
 */
enum salloc_bucket_state {
    bucket_empty,
    bucket_partial,
    bucket_full,
};

struct salloc_bucket {
    // Control fields
    struct salloc_bucket *prev;
    struct salloc_bucket *next;
    enum salloc_bucket_state state;
    
    // Backlink to the obj struct
    struct salloc_obj *obj;
    
    // Total number of entries and avail entries
    int entry_count;
    int avail_count;
    
    // Pointer to the first avail block
    struct salloc_magic_block *block;
};

struct salloc_bucket_list {
    struct salloc_bucket *next;
    int count;
};


/*
 * Salloc object
 */
struct salloc_obj {
    int obj_id;
    
    // Sizes
    size_t struct_size;
    size_t block_size;
    
    // Alignment
    size_t alignment;
    ulong block_start_offset;
    
    // Constructor/Destructor
    void (*constructor)(void* entry);
    void (*destructor)(void* entry);
    
    // Bucket info
    int bucket_block_count;
    
    // Buckets
    //  Note that we only have partial list since empty buckets are freed
    //  immediately, and full buckets are dangling, they will be put back to
    //  the partial list when they become partial
    struct salloc_bucket_list partial;
    
    // The lock that protects the entire salloc obj
    kthread_mutex_t obj_mutex;
};


struct salloc_obj_chunk {
    // Total number of entries and avail entries
    int entry_count;
    int avail_count;
    
    struct salloc_obj_chunk *next;
    struct salloc_obj *entries;
};


static struct salloc_obj_chunk *obj_chunk;

static int cur_obj_id = 0;
static kthread_mutex_t obj_chunk_mutex;


/*
 * Salloc object
 */
static struct salloc_obj *get_obj(int id, struct salloc_obj_chunk **chunk_out)
{
    struct salloc_obj_chunk *cur_chunk = obj_chunk;
    struct salloc_obj *obj = NULL;
    
    while (id >= cur_chunk->entry_count) {
        //assert(cur_chunk->next);
        
        id -= cur_chunk->entry_count;
        cur_chunk = cur_chunk->next;
    }
    
    obj = &cur_chunk->entries[id];
    if (chunk_out) {
        *chunk_out = cur_chunk;
    }
    
    return obj;
}

static struct salloc_obj_chunk *alloc_obj_chunk()
{
    struct salloc_obj_chunk *chunk = (struct salloc_obj_chunk *)halloc();
    if (!chunk) {
        return NULL;
    }
    
    // Initialize the obj page
    chunk->next = NULL;
    chunk->entries = (struct salloc_obj *)((ulong)chunk + sizeof(struct salloc_obj_chunk));
    
    // Calculate entry count
    int entry_count = (HALLOC_CHUNK_SIZE - sizeof(struct salloc_obj_chunk)) / sizeof(struct salloc_obj);
    chunk->entry_count = entry_count;
    chunk->avail_count = entry_count;
    
    return chunk;
}

static int alloc_obj(struct salloc_obj **obj_out, struct salloc_obj_chunk **chunk_out)
{
    int obj_id;
    struct salloc_obj *obj;
    struct salloc_obj_chunk *chunk, *new_chunk;
    
    kthread_mutex_lock(&obj_chunk_mutex);
    
    // Get the last allocated chunk
    obj = get_obj(cur_obj_id, &chunk);
    
    // Create a new obj chunk if there is no entries avail in the current chunk
    if (!chunk->avail_count) {
        new_chunk = alloc_obj_chunk();
        if (!new_chunk) {
            kthread_mutex_unlock(&obj_chunk_mutex);
            return 0;
        }
        
        chunk->next = new_chunk;
    }
    
    // New object ID
    obj_id = ++cur_obj_id;
    
    obj = get_obj(obj_id, &chunk);
    obj->obj_id = obj_id;
    chunk->avail_count--;
    
    atomic_membar();
    kthread_mutex_unlock(&obj_chunk_mutex);
    
    if (obj_out) {
        *obj_out = obj;
    }
    
    if (chunk_out) {
        *chunk_out = chunk;
    }
    
    return obj_id;
}

int salloc_create(size_t size, size_t align, salloc_callback_t construct, salloc_callback_t destruct)
{
    // Allocate a new obj
    struct salloc_obj *obj;
    int obj_id = alloc_obj(&obj, NULL);
    
    if (!obj_id) {
        return 0;
    }
    
    // Calculate alignment
    if (!align) {
        align = sizeof(unsigned long);
    }
    //assert(align <= ALIGN_MAX && align >= ALIGN_MIN);
    
    // Block size
    size_t block_size = size + sizeof(struct salloc_magic_block);
    if (block_size % align) {
        block_size /= align;
        block_size++;
        block_size *= align;
    }
    
    // Start offset
    ulong start_offset = sizeof(struct salloc_bucket) + sizeof(struct salloc_magic_block);
    if (start_offset % align) {
        start_offset /= align;
        start_offset++;
        start_offset *= align;
    }
    start_offset -= sizeof(struct salloc_magic_block);
    
    // Recalculae block count
    int block_count = (HALLOC_CHUNK_SIZE - start_offset) / block_size;
    
    // Initialize the object
    obj->struct_size = size;
    obj->block_size = block_size;
    
    obj->alignment = align;
    obj->block_start_offset = start_offset;
    
    obj->constructor = construct;
    obj->destructor = destruct;
    
    obj->bucket_block_count = block_count;
    
    obj->partial.count = 0;
    obj->partial.next = NULL;
    
    // Init the lock
    kthread_mutex_init(&obj->obj_mutex);
    
//     // Echo
//     kprintf("\tSalloc object created\n");
//     kprintf("\t\tStruct size: %d\n", obj->struct_size);
//     kprintf("\t\tBlock size: %d\n", obj->block_size);
//     kprintf("\t\tAlignment: %d\n", obj->alignment);
//     kprintf("\t\tStart offset: %d\n", obj->block_start_offset);
//     kprintf("\t\tBucket block count: %d\n", obj->bucket_block_count);
//     kprintf("\t\tConstructor: %p\n", obj->constructor);
//     kprintf("\t\tDestructor: %p\n", obj->destructor);
    
    // Done
    return obj_id;
}


/*
 * Bucket
 */
static struct salloc_bucket *alloc_bucket(struct salloc_obj *obj)
{
    struct salloc_bucket *bucket = (struct salloc_bucket *)halloc();
    if (!bucket) {
        return NULL;
    }
    
    // Initialize the bucket header
    bucket->obj = obj;
    
    bucket->next = NULL;
    bucket->prev = NULL;
    bucket->state = bucket_empty;
    
    bucket->entry_count = obj->bucket_block_count;
    bucket->avail_count = obj->bucket_block_count;
    
    bucket->block = NULL;
    
    // Initialize all the blocks
    int i;
    for (i = 0; i < bucket->entry_count; i++) {
        struct salloc_magic_block *block = (struct salloc_magic_block *)((ulong)bucket + obj->block_start_offset + i * obj->block_size);
        //kprintf("block: #%d/%d@%p\n", i, bucket->entry_count, block);
        
        // Setup block header
        block->bucket = bucket;
        
        // Push the block to the block list
        block->next = bucket->block;
        bucket->block = block;
    }
    
    // Done
    return bucket;
}

static void free_bucket(struct salloc_bucket *bucket)
{
    hfree(bucket);
}

static void insert_bucket(struct salloc_bucket_list *list, struct salloc_bucket *bucket)
{
    bucket->next = list->next;
    bucket->prev = NULL;
    
    if (list->next) {
        list->next->prev = bucket;
    }
    list->next = bucket;
    
    list->count++;
}

static void remove_bucket(struct salloc_bucket_list *list, struct salloc_bucket *bucket)
{
    list->count--;
    
    if (bucket->prev) {
        bucket->prev->next = bucket->next;
    } else {
        list->next = bucket->next;
    }
    
    if (bucket->next) {
        bucket->next->prev = bucket->prev;
    }
}


/*
 * Actual alloc and free
 */
void *salloc(int obj_id)
{
    struct salloc_obj *obj = get_obj(obj_id, NULL);
    struct salloc_bucket *bucket = NULL;
    
    // Lock the salloc obj
    kthread_mutex_lock(&obj->obj_mutex);
    
    // If there is no partial bucket avail, we need to allocate a new one
    if (!obj->partial.count) {
        bucket = alloc_bucket(obj);
    } else {
        bucket = obj->partial.next;
    }
    
    // If we were not able to obtain a usable bucket, then the fail
    if (!bucket) {
        kthread_mutex_unlock(&obj->obj_mutex);
        return NULL;
    }
    
    // Get the first avail block in the bucket;
    struct salloc_magic_block *block = bucket->block;
    
    // Pop the block from the bucket
    bucket->block = block->next;
    bucket->avail_count--;
    block->next = NULL;
    
    // Setup the block
    block->bucket = bucket;
    
    // Change the bucket state and remove it from the partial list if necessary
    if (!bucket->avail_count) {
        bucket->state = bucket_full;
        remove_bucket(&obj->partial, bucket);
    } else if (bucket->avail_count == bucket->entry_count - 1) {
        bucket->state = bucket_partial;
        insert_bucket(&obj->partial, bucket);
    }
    
    // Unlock the salloc obj
    kthread_mutex_unlock(&obj->obj_mutex);
    
    // Calculate the final addr of the allocated struct
    void *ptr = (void *)((ulong)block + sizeof(struct salloc_magic_block));
    
    // Call the constructor
    if (obj->constructor) {
        obj->constructor(ptr);
    }
    
    return ptr;
}

void sfree(void *ptr)
{
    // Obtain the magic block
    struct salloc_magic_block *block = (struct salloc_magic_block *)((ulong)ptr - sizeof(struct salloc_magic_block));
    
    // Obtain the bucket and obj
    struct salloc_bucket *bucket = block->bucket;
    struct salloc_obj *obj = bucket->obj;
    
    // Call the destructor
    if (obj->destructor) {
        obj->destructor(ptr);
    }
    
    // Lock the salloc obj
    kthread_mutex_lock(&obj->obj_mutex);
    
    // Push the block back to the bucket
    block->next = bucket->block;
    bucket->block = block;
    bucket->avail_count++;
    
    // Change the block state and add it to/remove it from the partil list if necessary
    if (bucket->avail_count == bucket->entry_count) {
        bucket->state = bucket_empty;
        remove_bucket(&obj->partial, bucket);
        free_bucket(bucket);
    } else if (1 == bucket->avail_count) {
        bucket->state = bucket_partial;
        insert_bucket(&obj->partial, bucket);
    }
    
    // Unlock the salloc obj
    kthread_mutex_unlock(&obj->obj_mutex);
}


/*
 * Initialize
 */
void init_salloc()
{
    obj_chunk = alloc_obj_chunk();
    kthread_mutex_init(&obj_chunk_mutex);
}
