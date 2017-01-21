#include "common/include/data.h"
#include "common/include/coreimg.h"
#include "common/include/proc.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/assert.h"
#include "klibc/include/time.h"
#include "klibc/include/sys.h"
#include "system/include/urs.h"


struct coreimg_data {
    unsigned long size;
    u8 *data;
};

struct coreimg_sub {
    unsigned long count;
    hash_t *entries;
};

struct coreimg_node {
    unsigned long id;
    char *name;
    struct coreimg_node *parent;
    
    unsigned long ref_count;
    unsigned long open_count;
    
    struct coreimg_sub sub;
    struct coreimg_data data;
    
    time_t create_time;
    time_t read_time;
    time_t write_time;
    time_t list_time;
    time_t change_time;
};

struct coreimg_open {
    unsigned long id;
    struct coreimg_node *node;
    
    unsigned long data_pos;
    unsigned long sub_pos;
};


static struct coreimg_header *header;

static struct coreimg_node root_node;
static struct coreimg_node *file_nodes;

static int open_salloc_id;


/*
 * Node Hash
 */
static unsigned int urs_hash_func(void *key, unsigned int size)
{
    char *str = (char *)key;
    unsigned int k = 0;
    
    do {
        k += (unsigned int)(*str);
    } while (*str++);
    
    return k % size;
}

static int urs_hash_cmp(void *cmp_key, void *node_key)
{
    char *cmp_ch = (char *)cmp_key;
    char *node_ch = (char *)node_key;
    
    return strcmp(cmp_ch, node_ch);
}


/*
 * Helpers
 */
static struct coreimg_node *get_node_by_id(unsigned long super_id, unsigned long node_id)
{
    return (struct coreimg_node *)node_id;
}

static struct coreimg_open *get_open_by_id(unsigned long open_id)
{
    return (struct coreimg_open *)open_id;
}

static struct coreimg_open *create_open(unsigned long super_id, unsigned long node_id)
{
    struct coreimg_node *node = get_node_by_id(super_id, node_id);
    if (!node) {
        return NULL;
    }
    
    struct coreimg_open *open = (struct coreimg_open *)salloc(open_salloc_id);
    
    open->id = (unsigned long)open;
    open->node = node;
    
    open->data_pos = 0;
    open->sub_pos = 0;
    
    return open;
}


/*
 * Node
 */
static int lookup(unsigned long super_id, unsigned long node_id, const char *name, int *is_link,
                  unsigned long *next_id, void *buf, unsigned long count, unsigned long *actual)
{
    struct coreimg_node *node = NULL, *next = NULL;
    
//     kprintf("Lookup received, super_id: %p, node_id: %lu, name: %s\n", super_id, node_id, name);
    
    // Find out the next node
    if (node_id) {
        node = get_node_by_id(super_id, node_id);
        assert(node);
        
        node->ref_count--;
        if (!node->sub.count) {
            return 0;
        }
        
        next = (struct coreimg_node *)hash_obtain(node->sub.entries, (void *)name);
        if (!next) {
            hash_release(node->sub.entries, (void *)name, next);
            return 0;
        }
    } else {
        node = &root_node;
        next = node;
    }
    
    // Set results
    next->ref_count++;
    
    if (is_link) {
        *is_link = 0;
    }
    
    if (next_id) {
        *next_id = next->id;
    }
    
    // Release hash table
    if (node_id) {
        hash_release(node->sub.entries, (void *)name, next);
    }
    
    return 0;
}

static int open(unsigned long super_id, unsigned long node_id, unsigned long *open_id)
{
    struct coreimg_node *node = get_node_by_id(super_id, node_id);
    if (!node) {
        return ENOENT;
    }
    
    struct coreimg_open *open = create_open(super_id, node_id);
    if (!open) {
        return EBADF;
    }
    
    if (open_id) {
        *open_id = open->id;
    }
    
    node->change_time = time();
    
    return EOK;
}

static int close(unsigned long super_id, unsigned long open_id)
{
    struct coreimg_open *open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    open->node->change_time = time();
    
    // FIXME: Lock needed
    open->node->open_count--;
    sfree(open);
    
    return EOK;
}

static unsigned long read_data_block(struct coreimg_data *data, unsigned long pos, u8 *buf, unsigned long count)
{
    int index = 0;
    unsigned long cur_pos = pos;
    
    while (buf && cur_pos < data->size && (unsigned long)index < count) {
        buf[index] = data->data[cur_pos];
        index++;
        cur_pos++;
    }
    
//     kprintf("data read: %s, size: %p\n", (char *)buf, index);
    return index;
}

static int read(unsigned long super_id, unsigned long open_id, void *buf, unsigned long count, unsigned long *actual)
{
    unsigned long result = 0;
    
    struct coreimg_open *open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    result = read_data_block(&open->node->data, (unsigned long)open->data_pos, buf, count);
    open->data_pos += result;
    
    if (actual) {
        *actual = result;
    }
    
    open->node->read_time = time();
    
    return 0;
}

static int seek_data(unsigned long super_id, unsigned long open_id, u64 offset, enum urs_seek_from from, u64 *newpos)
{
    unsigned long pos = 0;
    
    struct coreimg_open *open = NULL;
    struct coreimg_node *node = NULL;
    
    open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    node = open->node;
    if (!node) {
        return ECLOSED;
    }
    
    switch (from) {
    case seek_from_begin:
        pos = (unsigned long)offset;
        if (pos > node->data.size) {
            pos = node->data.size;
        }
        break;
    case seek_from_cur_fwd:
        pos = open->data_pos + (unsigned long)offset;
        if (pos > node->data.size) {
            pos = node->data.size;
        }
        break;
    case seek_from_cur_bwd:
        if (open->data_pos > (unsigned long)offset) {
            pos = open->data_pos - (unsigned long)offset;
        } else {
            pos = 0;
        }
        break;
    case seek_from_end:
        if (node->data.size > (unsigned long)offset) {
            pos = node->data.size - (unsigned long)offset;
        } else {
            pos = 0;
        }
        break;
    default:
        break;
    }
    
    open->data_pos = (u64)pos;
    if (newpos) {
        *newpos = (u64)pos;
    }
    
    return 0;
}

static int list(unsigned long super_id, unsigned long open_id, void *buf, unsigned long count, unsigned long *actual)
{
    struct coreimg_node *sub = NULL;
    unsigned long len = 0;
    int result = 0;
     
    struct coreimg_open *open = NULL;
    struct coreimg_node *node = NULL;
    
    open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    node = open->node;
    if (!node) {
        return ECLOSED;
    }
    
    // Sub entries have not yet been initialized
    if (!node->sub.entries) {
        return -1;
    }
    
    // No more entries
    if (open->sub_pos >= node->sub.count) {
        return -1;
    }
    
    // Obtain the sub entry
//     kprintf("to obtain at pos: %p, entries: %p, count: %p\n", open->sub_pos, node->sub.entries, node->sub.count);
    sub = hash_obtain_at(node->sub.entries, open->sub_pos);
//     kprintf("obtained: %p\n", sub);
    
    // Copy the name
    if (buf) {
        len = strlen(sub->name) + 1;
        len = count > len ? len : count;
        memcpy(buf, sub->name, len);
        ((char *)buf)[len - 1] = '\0';
//         kprintf("entry name: %s\n", (char *)buf);
    }
    
    if (actual) {
        *actual = len;
    }
    
    open->sub_pos++;
    hash_release(node->sub.entries, NULL, sub);
    
    node->list_time = time();
    
    return result;
}

static int seek_list(unsigned long super_id, unsigned long open_id, u64 offset, enum urs_seek_from from, u64 *newpos)
{
    unsigned long pos = 0;
    
    struct coreimg_open *open = NULL;
    struct coreimg_node *node = NULL;
    
    open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    node = open->node;
    if (!node) {
        return ECLOSED;
    }
    
    switch (from) {
    case seek_from_begin:
        pos = (unsigned long)offset;
        if (pos > node->sub.count) {
            pos = node->sub.count;
        }
        break;
    case seek_from_cur_fwd:
        pos = open->sub_pos + (unsigned long)offset;
        if (pos > node->sub.count) {
            pos = node->sub.count;
        }
        break;
    case seek_from_cur_bwd:
        if (open->sub_pos > (unsigned long)offset) {
            pos = open->sub_pos - (unsigned long)offset;
        } else {
            pos = 0;
        }
        break;
    case seek_from_end:
        if (node->sub.count > (unsigned long)offset) {
            pos = node->sub.count - (unsigned long)offset;
        } else {
            pos = 0;
        }
        break;
    default:
        break;
    }
    
    open->sub_pos = (u64)pos;
    if (newpos) {
        *newpos = (u64)pos;
    }
    
    return 0;
}

static int stat(unsigned long super_id, unsigned long open_id, struct urs_stat *stat)
{
    struct coreimg_open *open = NULL;
    struct coreimg_node *node = NULL;
    
    open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    node = open->node;
    if (!node) {
        return ECLOSED;
    }
    
    if (stat) {
        stat->super_id = 0;
        stat->open_dispatch_id = open->id;
        
        stat->num_links = node->ref_count;
        stat->sub_count = node->sub.count;
        stat->data_size = node->data.size;
        stat->occupied_size = stat->data_size;
        
        stat->create_time = node->create_time;
        stat->read_time = node->read_time;
        stat->write_time = node->write_time;
        stat->list_time = node->list_time;
        stat->change_time = node->change_time;
    }
    
    return EOK;
}


/*
 * Initialization
 */
#define OP_FUNC(t, f)   ops.entries[t].type = ureg_func; ops.entries[t].func = f

void init_coreimgfs()
{
    int i;
    
    // First map coreimg into my addr space
    header = (struct coreimg_header *)kapi_kmap(kmap_coreimg);
    
    // Construct the root node
    root_node.id = (unsigned long)&root_node;
    root_node.name = strdup("/");
    root_node.parent = NULL;
    
    root_node.ref_count = 0;
    root_node.open_count = 0;
    
    root_node.data.size = 0;
    root_node.data.data = NULL;
    
    root_node.sub.count = header->file_count;
    root_node.sub.entries = hash_new(0, urs_hash_func, urs_hash_cmp);
    
    root_node.create_time = header->time_stamp;
    root_node.read_time = root_node.write_time = root_node.list_time = root_node.change_time = 0;
    
    // Construct file nodes
    struct coreimg_record *file_records = (struct coreimg_record *)((unsigned long)header + sizeof(struct coreimg_header));
    file_nodes = calloc(header->file_count, sizeof(struct coreimg_node));
    for (i = 0; i < header->file_count; i++) {
        struct coreimg_node *node = &file_nodes[i];
        
        node->id = (unsigned long)node;
        node->parent = &root_node;
        node->name = malloc(21);
        memzero(node->name, 21);
        memcpy(node->name, file_records[i].file_name, 20);
        
        node->data.size = file_records[i].length;
        node->data.data = (u8 *)((unsigned long)header + file_records[i].start_offset);
        
        node->sub.count = 0;
        node->sub.entries = NULL;
        
        node->create_time = header->time_stamp;
        node->read_time = node->write_time = node->list_time = node->change_time = 0;
        
        hash_insert(root_node.sub.entries, file_nodes[i].name, &file_nodes[i]);
    }
    
    // Register CoreimgFS
    open_salloc_id = salloc_create(sizeof(struct coreimg_open), 0, NULL, NULL);
    
    struct urs_reg_ops ops;
    unsigned long super_id = 0;
    
    OP_FUNC(uop_lookup, lookup);
    OP_FUNC(uop_open, open);
    OP_FUNC(uop_release, close);
    
    OP_FUNC(uop_read, read);
    OP_FUNC(uop_seek_data, seek_data);
    
    OP_FUNC(uop_list, list);
    OP_FUNC(uop_seek_list, seek_list);
    
    OP_FUNC(uop_stat, stat);
    
    // Register the FS
    super_id = urs_register("coreimg://", "coreimgfs", 0, &ops);
    assert(super_id);
}