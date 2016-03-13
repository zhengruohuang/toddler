#include "common/include/data.h"
#include "kernel/include/hal.h"


#define PALLOC_MAX_ORDER    10
#define PALLOC_MIN_ORDER    0
#define PALLOC_ORDER_COUNT  (PALLOC_MAX_ORDER - PALLOC_MIN_ORDER + 1)


struct palloc_node {
    union {
        ulong value;
        struct {
            ulong order: PAGE_BITS;
            ulong next : PFN_BITS;
        };
    };
} packedstruct;

struct palloc_bucket {
    ulong page_count;
    struct palloc_node *node_groups[PALLOC_ORDER_COUNT];
    struct palloc_bucket *next;
};

struct palloc_tag {
    int tag;
    struct palloc_bucket *bucket_head;
};

static struct palloc_tag tags[16];
