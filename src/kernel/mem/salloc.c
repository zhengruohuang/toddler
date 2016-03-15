/*
 * Structure Allocator
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"


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
    int bucket_page_count;
    int bucket_block_count;
    
    // Buckets
    //  Note that we only have partial list since empty buckets are freed
    //  immediately, and full buckets are dangling, they will be put back to
    //  the partial list when they become partial
    struct salloc_bucket_list partial;
};


struct salloc_obj_page {
    // Total number of entries and avail entries
    int entry_count;
    int avail_count;
    
    // ID base
    int obj_id_base;
    
    struct salloc_obj_page *next;
    struct salloc_obj *entries;
};


static struct salloc_obj_page *obj_page;
static int cur_obj_id = 0;


/*
 * Salloc object manipulation
 */
static struct salloc_obj *get_obj(int id)
{
    struct salloc_obj_page *cur_page = obj_page;
    
    while (id >= cur_page->entry_count) {
        assert(cur_page->next);
        
        id -= cur_page->entry_count;
        cur_page = cur_page->next;
    }
    
    struct salloc_obj *obj = &cur_page->entries[id];
    return obj;
}


/*
 * Bucket manipulation
 */
static struct salloc_bucket *alloc_bucket(struct salloc_obj *obj)
{
    struct salloc_bucket *bucket = (struct salloc_bucket *)PFN_TO_ADDR(palloc(obj->bucket_page_count));
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
    ulong addr = (ulong)bucket;
    pfree(ADDR_TO_PFN(addr));
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
 * Initialize struct allocator
 */
void init_salloc()
{
    kprintf("Initializing struct allocator\n");
    
    // Allocate a page for obj page
    obj_page = (struct salloc_obj_page *)PFN_TO_ADDR(palloc(1));
    
    // Initialize the obj page
    obj_page->obj_id_base = 0;
    obj_page->next = NULL;
    
    obj_page->entries = (struct salloc_obj *)((ulong)obj_page + sizeof(struct salloc_obj_page));
    
    // Calculate entry count
    int entry_count = (PAGE_SIZE - sizeof(struct salloc_obj_page)) / sizeof(struct salloc_obj);
    obj_page->entry_count = entry_count;
    obj_page->avail_count = entry_count;
}


/*
 * Create a salloc object
 */
int salloc_create(size_t size, size_t align, int count, salloc_callback_t construct, salloc_callback_t destruct)
{
    // Object ID
    cur_obj_id++;
    
    // Obtain the object and set obj ID
    struct salloc_obj *obj = get_obj(cur_obj_id);
    obj->obj_id = cur_obj_id;
    
    // Less avail entries in obj_page
    obj_page->avail_count--;
    
    // Calculate alignment
    if (!align) {
        align = ALIGN_DEFAULT;
    }
    assert(align <= ALIGN_MAX && align >= ALIGN_MIN);
    
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
    
    // Block count and page count
    if (!count) {
        count = 32;
    }
    ulong total_size = start_offset + block_size * count;
    
    // Calculate page count
    int page_count = total_size / PAGE_SIZE;
    if (total_size % PAGE_SIZE) {
        page_count++;
    }
    page_count = 0x1 << calc_palloc_order(page_count);
    
    // Recalculae block count
    int block_count = (page_count * PAGE_SIZE - start_offset) / block_size;
    
    // Initialize the object
    obj->struct_size = size;
    obj->block_size = block_size;
    
    obj->alignment = align;
    obj->block_start_offset = start_offset;
    
    obj->constructor = construct;
    obj->destructor = destruct;
    
    obj->bucket_page_count = page_count;
    obj->bucket_block_count = block_count;
    
    obj->partial.count = 0;
    obj->partial.next = NULL;
    
//     // Echo
//     kprintf("\tSalloc object created\n");
//     kprintf("\t\tStruct size: %d\n", obj->struct_size);
//     kprintf("\t\tBlock size: %d\n", obj->block_size);
//     kprintf("\t\tAlignment: %d\n", obj->alignment);
//     kprintf("\t\tStart offset: %d\n", obj->block_start_offset);
//     kprintf("\t\tBucket page count: %d\n", obj->bucket_page_count);
//     kprintf("\t\tBucket block count: %d\n", obj->bucket_block_count);
//     kprintf("\t\tConstructor: %p\n", obj->constructor);
//     kprintf("\t\tDestructor: %p\n", obj->destructor);
    
    // Done
    return cur_obj_id;
}


/*
 * Allocate and deallocate
 */
void *salloc(int obj_id)
{
    struct salloc_obj *obj = get_obj(obj_id);
    
    struct salloc_bucket *bucket = NULL;
    
    // If there is no partial bucket avail, we need to allocate a new one
    if (!obj->partial.count) {
        bucket = alloc_bucket(obj);
    } else {
        bucket = obj->partial.next;
    }
    
    // If we were not able to obtain a usable bucket, then the fail
    if (!bucket) {
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
}
