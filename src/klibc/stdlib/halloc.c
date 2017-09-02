#include "common/include/data.h"
#include "common/include/atomic.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"


#define BITS_PER_ENTRY      (sizeof(unsigned long) * 8)
#define BITMAP_ENTRY_COUNT  ((HALLOC_CHUNK_SIZE / sizeof(unsigned long)))


static kthread_mutex_t heap_mutex;

static unsigned long heap_start = 0;
static unsigned long heap_end = 0;

static int cur_last_bitmap_entry = 0;
static unsigned long *alloc_bitmap = NULL;


static int bit_test(int index, int bit)
{
    unsigned long map = alloc_bitmap[index];
    unsigned long mask = 0x1;
    mask <<= bit;
    return (map & mask) ? 1 : 0;
}

static void bit_set(int index, int bit)
{
    unsigned long mask = 0x1;
    mask <<= bit;
    alloc_bitmap[index] |= mask;
}

static void bit_clear(int index, int bit)
{
    unsigned long mask = 0x1;
    mask <<= bit;
    mask = ~mask;
    alloc_bitmap[index] &= mask;
}

static int find_first(int index, int one)
{
    int i;
    
    for (i = 0; i < BITS_PER_ENTRY; i++) {
        if (bit_test(index, i) == one) {
            return i;
        }
    }
}

static int find_last(int index, int one)
{
    int i;
    
    for (i = BITS_PER_ENTRY - 1; i >= 0; i--) {
        if (bit_test(index, i) == one) {
            return i;
        }
    }
}

static int entry_avail(int index)
{
    unsigned long mask = 0;
    mask = ~mask;
    return alloc_bitmap[index] != mask;
}

static int entry_inuse(int index)
{
    unsigned long mask = 0;
    return alloc_bitmap[index] != mask;
}


void init_halloc()
{
    int i;
    
    // Init the heap
    heap_start = kapi_get_heap_end();
    
    // Alloc the bitmap array
    kapi_grow_heap(HALLOC_CHUNK_SIZE);
    heap_end = heap_start + HALLOC_CHUNK_SIZE;
    alloc_bitmap = (unsigned long *)heap_start;
    
    // Initialize the bitmap
    for (i = 0; i < BITMAP_ENTRY_COUNT; i++) {
        alloc_bitmap[i] = 0;
    }
    
    bit_set(0, 0);
    cur_last_bitmap_entry = 0;
    
    // Init lock
    kthread_mutex_init(&heap_mutex);
}

static int resize_heap()
{
    int e = cur_last_bitmap_entry;
    int b = find_last(e, 1);
    unsigned long addr = heap_start + HALLOC_CHUNK_SIZE * (BITS_PER_ENTRY * e + b + 1);
    
    if (addr == heap_end) {
        return 1;
    }
    
    if (kapi_brk(addr)) {
        heap_end = addr;
        return 1;
    }
    
    return 0;
}

void *halloc()
{
    int e;
    int b;
    int found = 0;
    
    int prev_entry;
    unsigned long addr;
    void *result = NULL;
    
    kthread_mutex_lock(&heap_mutex);
    
    // Find the first avail entry
    for (e = 0; e < BITMAP_ENTRY_COUNT; e++) {
        if (entry_avail(e)) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        goto done;
    }
    
    // Find and set the first avail bit
    b = find_first(e, 0);
    bit_set(e, b);
    
    // Resize the heap
    prev_entry = cur_last_bitmap_entry;
    if (e > cur_last_bitmap_entry) {
        cur_last_bitmap_entry = e;
    }
    
    if (!resize_heap()) {
        bit_clear(e, b);
        cur_last_bitmap_entry = prev_entry;
        
        goto done;
    }
    
    // Calculate the final address
    addr = heap_start + HALLOC_CHUNK_SIZE * (BITS_PER_ENTRY * e + b);
    result = (void *)addr;
    
done:
    atomic_membar();
    kthread_mutex_unlock(&heap_mutex);
    
    return result;
}

void hfree(void *ptr)
{
    unsigned long addr = (unsigned long)ptr;
    unsigned long index = (addr - heap_start) / HALLOC_CHUNK_SIZE;
    int e = (int)(index / BITS_PER_ENTRY);
    int b = (int)(index % BITS_PER_ENTRY);
    
    kthread_mutex_lock(&heap_mutex);
    
    bit_clear(e, b);
    if (cur_last_bitmap_entry > e) {
        goto done;
    }
    
    // Resize the heap
    for (; e; e--) {
        if (entry_inuse(e)) {
            cur_last_bitmap_entry = e;
            break;
        }
    }
    
    resize_heap();
    
done:
atomic_membar();
    kthread_mutex_unlock(&heap_mutex);
}
