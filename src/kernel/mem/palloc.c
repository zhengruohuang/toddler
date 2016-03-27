/*
 * Page Frame Allocator
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"


#define PALLOC_ORDER_BITS   4
#define PALLOC_MAX_ORDER    15
#define PALLOC_MIN_ORDER    0
#define PALLOC_ORDER_COUNT  (PALLOC_MAX_ORDER - PALLOC_MIN_ORDER + 1)

#define PALLOC_DEFAULT_TAG  0
#define PALLOC_BUCKET_COUNT 16
#define PALLOC_DUMMY_BUCKET 15
#define PALLOC_BUCKET_BITS  4


struct palloc_node {
    union {
        ulong value;
        struct {
            ulong order     : PALLOC_ORDER_BITS;
            ulong tag       : PALLOC_BUCKET_BITS;
            ulong alloc     : 1;
            ulong has_next  : 1;
            ulong avail     : 1;
            ulong next      : PFN_BITS;
        };
    };
} packedstruct;

struct palloc_bucket {
    int bucket_tag;
    
    ulong total_pages;
    ulong avail_pages;
    
    struct palloc_node buddies[PALLOC_ORDER_COUNT];
    
    spinlock_t lock;
};


static struct palloc_node *nodes;
static struct palloc_bucket buckets[PALLOC_BUCKET_COUNT];


/*
 * Node manipulation
 */
static struct palloc_node *get_node_by_paddr(ulong paddr)
{
    if (paddr >= hal->paddr_space_end) {
        return NULL;
    }
    
    return &nodes[ADDR_TO_PFN(paddr)];
}

static struct palloc_node *get_node_by_pfn(ulong pfn)
{
    if (pfn >= ADDR_TO_PFN(hal->paddr_space_end)) {
        return NULL;
    }
    
    return &nodes[pfn];
}

static void insert_node(ulong pfn, int tag, int order)
{
    struct palloc_node *node = get_node_by_pfn(pfn);
    
    node->next = buckets[tag].buddies[order].next;
    node->has_next = buckets[tag].buddies[order].has_next;
    
    buckets[tag].buddies[order].has_next = 1;
    buckets[tag].buddies[order].next = pfn;
}

static void remove_node(ulong pfn, int tag, int order)
{
    ulong cur_pfn = buckets[tag].buddies[order].next;
    struct palloc_node *cur = get_node_by_pfn(cur_pfn);
    struct palloc_node *prev = NULL;

    // If the node is the first one
    if (cur_pfn == pfn) {
        buckets[tag].buddies[order].has_next = cur->has_next;
        buckets[tag].buddies[order].next = cur->next;
        
        cur->has_next = 0;
        cur->next = 0;
        
        return;
    }
    
    // Otherwise we have to go through the list
    assert(cur->has_next);
    
    do {
        prev = cur;
        cur_pfn = cur->next;
        cur = get_node_by_pfn(cur_pfn);
        
        if (cur_pfn == pfn) {
            prev->has_next = cur->has_next;
            prev->next = cur->next;
            
            cur->has_next = 0;
            cur->next = 0;
            
            return;
        }
    } while (cur->has_next);
    
    panic("Unable to remove node from list, pfn: %p, tag: %d, order: %d\n", pfn, tag, order);
}


/*
 * Helper functions
 */
int calc_palloc_order(int count)
{
    int order;
    for (order = PALLOC_MIN_ORDER; order <= PALLOC_MAX_ORDER; order++) {
        if ((0x1 << order) >= count) {
            return order;
        }
    }
    
    panic("Too many pages to allocate: %d", count);
    return -1;
}

void buddy_print()
{
    int tag;
    for (tag = 0; tag < PALLOC_BUCKET_COUNT; tag++) {
        if (
            tag == PALLOC_DUMMY_BUCKET ||
            0 == buckets[tag].avail_pages
        ) {
            continue;
        }
        
        kprintf("\tBuddy in Bucket #%d\n", tag);
        
        int order;
        for (order = PALLOC_MIN_ORDER; order <= PALLOC_MAX_ORDER; order++) {
            int count = 0;
            
            int has_next = buckets[tag].buddies[order].has_next;
            struct palloc_node *cur = get_node_by_pfn(buckets[tag].buddies[order].next);
            
            while (has_next) {
                count++;
                
                has_next = cur->has_next;
                cur = get_node_by_pfn(cur->next);
            }
            
            kprintf("\t\tOrder: %d, Count: %d\n", order, count);
        }
    }
}


/*
 * Initialization
 */
static void init_bucket(ulong start, ulong len, int tag)
{
    kprintf("\tInitializing bucket, start: %p, len: %p, tag: %d\n", start, len, tag);
    assert(tag < PALLOC_BUCKET_COUNT && tag != PALLOC_DUMMY_BUCKET);
    
    // Update page count
    ulong page_count = len / PAGE_SIZE;
    buckets[tag].total_pages += page_count;
    buckets[tag].avail_pages += page_count;
    
    // Setup the buddy system
    int order;
    
    ulong cur_addr = start;
    ulong end = start + len;
    
    while (cur_addr < end) {
        for (order = PALLOC_MAX_ORDER; order >= PALLOC_MIN_ORDER; order--) {
            ulong order_size = ((ulong)0x1 << order) * PAGE_SIZE;
            
            // We found the correct buddy size
            if (
                0 == cur_addr % order_size &&
                cur_addr + order_size <= end
            ) {
                // Obtain the node
                struct palloc_node *node = get_node_by_paddr(cur_addr);
                node->order = order;
                node->alloc = 0;
                node->tag = tag;
                node->avail = 1;
                node->next = 0;
                node->has_next = 0;
                
                // Insert the chunk into the buddy list
                insert_node(ADDR_TO_PFN(cur_addr), tag, order);
                kprintf("\t\tBuddy inserted, addr: %p, size: %p, order: %d\n", cur_addr, order_size, order);
                
                cur_addr += order_size;
                break;
            }
        }
    }
}

void init_palloc()
{
    kprintf("Initializing page frame allocator\n");
    
    // Initialize bucket list
    int j, k;
    for (j = 0; j < PALLOC_BUCKET_COUNT; j++) {
        buckets[j].bucket_tag = j;
        buckets[j].total_pages = 0;
        buckets[j].avail_pages = 0;
        
        for (k = 0; k < PALLOC_ORDER_COUNT; k++) {
            buckets[j].buddies[k].value = 0;
        }
        
        spin_init(&buckets[j].lock);
    }
    
    // Calculate total number of nodes - 1 page needs 1 node
    ulong node_count = hal->paddr_space_end / PAGE_SIZE;
    ulong node_size = node_count * sizeof(struct palloc_node);
    kprintf("\tPalloc node size: %d KB, pages: %d\n", node_size / 1024, node_size / PAGE_SIZE);
    
    // Allocate and reserve memory
    nodes = (struct palloc_node *)hal->free_mem_start_addr;
    reserve_pfndb_mem(hal->free_mem_start_addr, node_size);
    hal->free_mem_start_addr += node_size;
    
    // Initialize all nodes
    ulong i;
    for (i = 0; i < node_count; i++) {
        nodes[i].avail = 0;
        nodes[i].tag = PALLOC_DUMMY_BUCKET;
    }
    
    // Go through PFN database to construct tags array
    struct pfndb_entry *entry = get_pfn_entry_by_paddr(0);
    int cur_tag = entry->tag;
    int recording = 0;
    ulong cur_bucket_start = 0;
    
    for (i = 0; i < hal->paddr_space_end; i += PAGE_SIZE) {
        entry = get_pfn_entry_by_paddr(i);
        
        // End of a bucket
        if (
            recording &&
            (!entry->usable || entry->inuse || cur_tag != entry->tag)
        ) {
            init_bucket(cur_bucket_start, i - cur_bucket_start, cur_tag);
            recording = 0;
        }
        
        // Start of a bucket
        else if (
            !recording &&
            (entry->usable && !entry->inuse && entry->tag != PALLOC_DUMMY_BUCKET)
        ) {
            cur_bucket_start = i;
            cur_tag = entry->tag;
            recording = 1;
        }
    }
    
    // Take care of the last bucket
    if (recording) {
        init_bucket(cur_bucket_start, hal->paddr_space_end - cur_bucket_start, cur_tag);
    }
    
    // Print the buddy
    buddy_print();
}

/*
 * Buddy
 */
static int buddy_split(int order, int tag)
{
    assert(tag < PALLOC_BUCKET_COUNT && tag != PALLOC_DUMMY_BUCKET);
    assert(order <= PALLOC_MAX_ORDER && order > PALLOC_MIN_ORDER);
    
    // Split higher order buddies if necessary
    if (!buckets[tag].buddies[order].has_next) {
        // If this is the highest order, then fail
        if (order == PALLOC_MAX_ORDER) {
            kprintf("Unable to split buddy\n");
            return -1;
        }
        
        if (-1 == buddy_split(order + 1, tag)) {
            return -1;
        }
    }
    
    // First obtain the palloc node
    ulong pfn = buckets[tag].buddies[order].next;
    struct palloc_node *node = get_node_by_pfn(pfn);
    
    // Remove the node from the list
    buckets[tag].buddies[order].next = node->next;
    buckets[tag].buddies[order].has_next = node->has_next;
    
    // Obtain the other node
    ulong pfn2 = pfn + ((ulong)0x1 << (order - 1));
    struct palloc_node *node2 = get_node_by_pfn(pfn2);
    
    // Setup the two nodes
    node->alloc = 0;
    node->order = order - 1;
    node->tag = tag;
    node->avail = 1;
    node->has_next = 0;
    node->next = 0;
    
    node2->alloc = 0;
    node2->order = order - 1;
    node2->tag = tag;
    node2->avail = 1;
    node2->has_next = 0;
    node2->next = 0;
    
    // Insert the nodes into the lower order list
    insert_node(pfn, tag, order - 1);
    insert_node(pfn2, tag, order - 1);
    
    return 0;
}

static void buddy_combine(ulong pfn)
{
    // Obtain the node
    struct palloc_node *node = get_node_by_pfn(pfn);
    int tag = node->tag;
    int order = node->order;
    int order_count = 0x1 << order;
    
    // If we already have the highest order, then we are done
    if (order == PALLOC_MAX_ORDER) {
        return;
    }
    
    // Get some info of the higher order
    int higher = order + 1;
    ulong higher_size = ((ulong)0x1 << higher) * PAGE_SIZE;
    ulong higher_pfn = 0;
    ulong other_pfn = 0;
    ulong cur_addr = PFN_TO_ADDR(pfn);
    
    // Check the other node to see if they can form a buddy
    if (0 == cur_addr % higher_size) {
        higher_pfn = pfn;
        other_pfn = pfn + order_count;
        
    } else {
        higher_pfn = pfn - order_count;
        other_pfn = pfn - order_count;
    }
    struct palloc_node *other_node = get_node_by_pfn(other_pfn);
    
    // If the other node doesn't belong to the same bucket as current node,
    // or the other node is in use, then we are done, there's no way to combine
    if (
        !other_node || !other_node->avail || other_node->tag != tag ||
        other_node->alloc || order != other_node->order
    ) {
        return;
    }
    
    // Remove both nodes from the list
    remove_node(pfn, tag, order);
    remove_node(other_pfn, tag, order);
    
    // Setup the two nodes
    node->has_next = 0;
    node->next = 0;
    node->order = higher;
    node->tag = tag;
    node->avail = 1;
    other_node->has_next = 0;
    other_node->next = 0;
    other_node->order = higher;
    other_node->tag = tag;
    other_node->avail = 1;
    
    // Insert them into the higher order list
    insert_node(higher_pfn, tag, higher);
    
    // Combine the higiher order
    buddy_combine(higher_pfn);
}


/*
 * Alloc and free
 */
ulong palloc_tag(int count, int tag)
{
    assert(tag < PALLOC_BUCKET_COUNT && tag != PALLOC_DUMMY_BUCKET);
    int order = calc_palloc_order(count);
    int order_count = 0x1 << order;
    
    // See if this bucket has enough pages to allocate
    if (buckets[tag].avail_pages < order_count) {
        return -1;
    }
    
    // If this is the highest order, then we are not able to allocate the pages
    if (order == PALLOC_MAX_ORDER) {
        return -1;
    }
    
    // Lock the bucket
    spin_lock_int(&buckets[tag].lock);
    
    // Split higher order buddies if necessary
    if (!buckets[tag].buddies[order].has_next) {
        if (-1 == buddy_split(order + 1, tag)) {
            kprintf("Unable to split buddy");
            
            spin_unlock_int(&buckets[tag].lock);
            return -1;
        }
    }
    
    // Now we are safe to allocate, first obtain the palloc node
    ulong pfn = buckets[tag].buddies[order].next;
    struct palloc_node *node = get_node_by_pfn(pfn);
    
    // Remove the node from the list
    buckets[tag].buddies[order].next = node->next;
    buckets[tag].buddies[order].has_next = node->has_next;
    
    // Mark the node as allocated
    node->alloc = 1;
    node->has_next = 0;
    node->next = 0;
    buckets[tag].avail_pages -= order_count;
    
    // Unlock the bucket
    spin_unlock_int(&buckets[tag].lock);
    
    return pfn;
}

ulong palloc(int count)
{
    ulong result = palloc_tag(count, PALLOC_DEFAULT_TAG);
    if (result) {
        return result;
    }
    
    int i;
    for (i = 0; i < PALLOC_BUCKET_COUNT; i++) {
        if (i != PALLOC_DEFAULT_TAG && i != PALLOC_DUMMY_BUCKET) {
            result = palloc_tag(count, PALLOC_DEFAULT_TAG);
            if (result) {
                return result;
            }
        }
    }
    
    return 0;
}

int pfree(ulong pfn)
{
    // Obtain the node
    struct palloc_node *node = get_node_by_pfn(pfn);
    
    // Get tag and order
    int tag = node->tag;
    int order = node->order;
    int order_count = 0x1 << order;
    
    // Setup the node
    node->alloc = 0;
    
    // Lock the bucket
    spin_lock_int(&buckets[tag].lock);
    
    // Insert the node back to the list
    if (buckets[tag].buddies[order].value) {
        node->next = buckets[tag].buddies[order].next;
        node->has_next = 1;
    } else {
        node->next = 0;
        node->has_next = 0;
    }
    buckets[tag].buddies[order].has_next = 1;
    buckets[tag].buddies[order].next = pfn;
    
    // Setup the bucket
    buckets[tag].avail_pages += order_count;
    
    // Combine the buddy system
    buddy_combine(pfn);
    
    // Unlock the bucket
    spin_unlock_int(&buckets[tag].lock);
    
    return order_count;
}


/*
 * Testing
 */
#define PALLOC_TEST_ORDER_COUNT 8
#define PALLOC_TEST_PER_ORDER   10
#define PALLOC_TEST_LOOPS       5

void test_palloc()
{
    kprintf("Testing palloc\n");
    
    int i, j, k;
    ulong results[PALLOC_TEST_ORDER_COUNT][PALLOC_TEST_PER_ORDER];
        
    for (k = 0; k < PALLOC_TEST_LOOPS; k++) {
        
        
        for (i = 0; i < PALLOC_TEST_ORDER_COUNT; i++) {
            for (j = 0; j < PALLOC_TEST_PER_ORDER; j++) {
                int count = 0x1 << i;
                //kprintf("Allocating count: %d, index: %d", count, j);
                ulong pfn = palloc(count);
                results[i][j] = pfn;
                //kprintf(", PFN: %p\n", pfn);
            }
        }
        
        for (i = 0; i < PALLOC_TEST_ORDER_COUNT; i++) {
            for (j = 0; j < PALLOC_TEST_PER_ORDER; j++) {
                ulong pfn = results[i][j];
                if (pfn) {
                    //int count = 0x1 << i;
                    //kprintf("Freeing count: %d, index: %d, PFN: %p\n", count, j, pfn);
                    pfree(pfn);
                }
            }
        }
    }
    
    //buddy_print();
    
    kprintf("Successfully passed the test!\n");
}
