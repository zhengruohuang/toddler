#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/assert.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"
#include "system/include/urs.h"


#define RAMFS_BLOCK_SIZE    512


struct ramfs_block {
    struct ramfs_block *prev;
    struct ramfs_block *next;
    u8 data[RAMFS_BLOCK_SIZE];
};

struct ramfs_data {
    unsigned long block_count;
    unsigned long size;
    unsigned long pos;
    
    struct ramfs_block *head;
    struct ramfs_block *tail;
    
    unsigned long cur_offset;
    struct ramfs_block *cur;
};

struct ramfs_sub {
    unsigned long count;
    unsigned long pos;
    hash_t *entries;
};

struct ramfs_node {
    unsigned long id;
    char *name;
    unsigned long ref_count;
    
    struct ramfs_node *parent;
    struct ramfs_data data;
    struct ramfs_sub sub;
};


static unsigned long block_salloc_id;
static unsigned long node_salloc_id;
static hash_t *ramfs_table;


/*
 * Init RAM FS
 */
void init_ramfs()
{
    ramfs_table = hash_new(0, NULL, NULL);
    node_salloc_id = salloc_create(sizeof(struct ramfs_node), 0, NULL, NULL);
    block_salloc_id = salloc_create(sizeof(struct ramfs_block), 0, NULL, NULL);
}


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
 * Data
 */
static unsigned long read_data_block(struct ramfs_data *data, u8 *buf, unsigned long count)
{
    unsigned long index = 0;
    
    while (data->pos < data->size && index < count && data->cur) {
        // Copy data
        buf[index] = data->cur->data[data->cur_offset];
        index++;
        data->cur_offset++;
        data->pos++;
        
        // Move to next data block
        if (data->cur_offset == RAMFS_BLOCK_SIZE) {
            data->cur = data->cur->next;
            data->cur_offset = 0;
        }
    }
    
    return index;
}

static unsigned long write_data_block(struct ramfs_data *data, u8 *buf, unsigned long count)
{
    unsigned long index = 0;
    
    kprintf("write, count: %lu\n", count);
    
    while (index < count) {
        // Allocate a new block if necessary
        if (!data->cur) {
            data->cur = (struct ramfs_block *)salloc(block_salloc_id);
            data->cur_offset = 0;
            
            data->cur->next = NULL;
            data->cur->prev = data->tail;
            
            if (data->tail) {
                data->tail->next = data->cur;
            }
            
            data->tail = data->cur;
            if (!data->head) {
                data->head = data->cur;
            }
            
            data->block_count++;
        }
        
        // Copy data
        data->cur->data[data->cur_offset] = buf[index];
        index++;
        data->cur_offset++;
        data->pos++;
        data->size++;
        
        // Move to next data block
        if (data->cur_offset == RAMFS_BLOCK_SIZE) {
            data->cur = data->cur->next;
            data->cur_offset = 0;
        }
    }
    
    return index;
}

static void seek_data_block_begin(struct ramfs_data *data, unsigned long offset)
{
    if (offset > data->size) {
        offset = data->size;
    }
    
    data->pos = 0;
    data->cur_offset = 0;
    data->cur = data->head;
    
    while (offset) {
        if (offset >= RAMFS_BLOCK_SIZE) {
            offset -= RAMFS_BLOCK_SIZE;
            data->pos += RAMFS_BLOCK_SIZE;
            
            assert(data->cur);
            data->cur = data->cur->next;
        } else {
            data->pos += offset;
            data->cur_offset = offset;
            offset = 0;
        }
    }
}

static unsigned long seek_data_block(struct ramfs_data *data, unsigned long offset, enum urs_seek_from from)
{
    switch (from) {
    case seek_from_begin:
        seek_data_block_begin(data, offset);
        break;
    case seek_from_cur_fwd:
        seek_data_block_begin(data, data->pos + offset);
        break;
    case seek_from_cur_bwd:
        seek_data_block_begin(data, data->pos > offset ? data->pos - offset : 0);
        break;
    case seek_from_end:
        seek_data_block_begin(data, data->size > offset ? data->size - offset : 0);
        break;
    default:
        break;
    }
    
    return data->pos;
}


/*
 * Node
 */
static struct urs_super *get_super_by_id(unsigned long id)
{
    return (struct urs_super *)id;
}

static struct ramfs_node *get_node_by_id(unsigned long id)
{
    return (struct ramfs_node *)id;
}

static struct ramfs_node *create_node(const char *name, struct ramfs_node *parent)
{
    struct ramfs_node *node = (struct ramfs_node *)salloc(node_salloc_id);
    
    node->id = (unsigned long)node;
    node->name = strdup(name);
    node->ref_count = 1;
    
    node->parent = parent;
    
    node->data.block_count = 0;
    node->data.pos = 0;
    node->data.size = 0;
    node->data.head = NULL;
    node->data.tail = NULL;
    
    node->data.cur = NULL;
    node->data.cur_offset = 0;
    
    node->sub.count = 0;
    node->sub.pos = 0;
    node->sub.entries = NULL;
    
    return node;
}

static int lookup(unsigned long super_id, unsigned long node_id, const char *name, unsigned long *next_id)
{
    struct urs_super *super = get_super_by_id(super_id);
    struct ramfs_node *node = NULL, *next = NULL;
    
    kprintf("Lookup received, super_id: %p, node_id: %lu, name: %s\n", super_id, node_id, name);
    
    if (next_id) {
        *next_id = 0;
    }
    
    if (node_id) {
        node = get_node_by_id(node_id);
        if (!node->sub.count) {
            return 0;
        }
        
        next = (struct ramfs_node *)hash_obtain(node->sub.entries, (void *)name);
        if (!next) {
            return 0;
        }
    } else {
        node = (struct ramfs_node *)hash_obtain(ramfs_table, (void *)super_id);
        hash_release(ramfs_table, (void *)super_id, node);
        next = node;
        assert(next);
    }
    
    next->ref_count++;
    if (next_id) {
        *next_id = next->id;
    }
    
    if (node_id) {
        hash_release(node->sub.entries, (void *)name, next);
    }
    
    return 0;
}

static int release(unsigned long super_id, unsigned long node_id)
{
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
    
    node->ref_count--;
    return 0;
}

static int read(unsigned long super_id, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    unsigned long result = 0;
    
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
    
    result = read_data_block(&node->data, buf, count);
    if (actual) {
        *actual = result;
    }
    
    return 0;
}

static int write(unsigned long super_id, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    unsigned long result = 0;
    
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
    
    result = write_data_block(&node->data, buf, count);
    if (actual) {
        *actual = result;
    }
    
    return 0;
}

static int seek_data(unsigned long super_id, unsigned long node_id, unsigned long offset, enum urs_seek_from from, unsigned long *newpos)
{
    unsigned long result = 0;
    
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
    
    result = seek_data_block(&node->data, offset, from);
    if (newpos) {
        *newpos = result;
    }
    
    return 0;
}

static int list(unsigned long super_id, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    struct ramfs_node *sub = NULL;
    unsigned long len = 0;
    int result = 0;
     
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
    
    if (!buf) {
        if (actual) {
            *actual = 0;
        }
        return 0;
    }
    
    sub = hash_obtain_at(node->sub.entries, node->sub.pos);
    len = strlen(sub->name) + 1;
    len = count > len ? len : count;
    memcpy(buf, sub->name, len);
    ((char *)buf)[len] = '\0';
    
    if (actual) {
        *actual = len;
    }
    
    node->sub.pos++;
    if (node->sub.pos >= node->sub.count) {
        node->sub.pos = 0;
        result = -1;
    }
    
    hash_release(node->sub.entries, NULL, sub);
    
    return result;
}

static unsigned long seek_list(unsigned long super_id, unsigned long node_id, unsigned long offset, enum urs_seek_from from)
{
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
    
    switch (from) {
    case seek_from_begin:
        node->sub.pos = offset < node->sub.count ? offset : node->sub.count - 1;
        break;
    case seek_from_cur_fwd:
        node->sub.pos = node->sub.pos + offset < node->sub.count ? node->sub.pos + offset : node->sub.count - 1;
        break;
    case seek_from_cur_bwd:
        node->sub.pos = offset < node->sub.pos ? node->sub.pos - offset : 0;
        break;
    case seek_from_end:
        node->sub.pos = offset < node->sub.count ? node->sub.count - offset - 1 : 0;
        break;
    default:
        break;
    }
}

static int create(unsigned long super_id, unsigned long node_id, char *name)
{
    struct ramfs_node *sub = NULL;
    
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
    
    sub = create_node(name, node);
    if (!node->sub.entries) {
        node->sub.entries = hash_new(0, urs_hash_func, urs_hash_cmp);
    }
    
    if (hash_insert(node->sub.entries, name, sub)) {
        sfree(sub);
        return -2;
    };
    
    node->sub.count++;
    
    return 0;
}

static int remove(unsigned long super_id, unsigned long node_id)
{
    return 0;
}

static int rename(unsigned long super_id, unsigned long node_id, char *name)
{
    struct ramfs_node *parent = NULL;
    
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
    
    parent = node->parent;
    if (!parent) {
        return -2;
    }
    
    hash_remove(parent->sub.entries, node->name);
    if (hash_insert(parent->sub.entries, name, node)) {
        free(node->name);
        node->name = strdup(name);
    } else {
        hash_insert(parent->sub.entries, node->name, node);
    }
    
    return 0;
}


/*
 * Super
 */
#define REGISTER_OP(type, func) urs_register_op(super_id, type, func, NULL, 0, 0, 0)

int register_ramfs(char *path)
{
    struct ramfs_node *root = NULL;
    
    unsigned long super_id = urs_register(path);
    if (!super_id) {
        return -2;
    }
    
    root = create_node("/", NULL);
    if (!root) {
        return -3;
    }
    
    hash_insert(ramfs_table, (void *)super_id, root);
    
    // Register operations
    REGISTER_OP(uop_lookup, lookup);
    REGISTER_OP(uop_release, release);
    
    REGISTER_OP(uop_read, read);
    REGISTER_OP(uop_write, write);
    REGISTER_OP(uop_seek_data, seek_data);
    
    REGISTER_OP(uop_list, list);
    REGISTER_OP(uop_seek_list, seek_list);
    
    REGISTER_OP(uop_create, create);
    REGISTER_OP(uop_remove, remove);
    REGISTER_OP(uop_rename, rename);
    
    return 0;
}


/*
 * Tests
 */
void test_ramfs()
{
    int err = 0;
    unsigned long size = 0;
    char *str = "this is a test string!";
    char buf[128];
    
    kprintf("To register RAM FS @ /test/ramfs\n");
    register_ramfs("/test/ramfs");
    kprintf("Registered!");
    
    kprintf("To open URS node");
    unsigned long f = urs_open_node("/test/ramfs", 0, 0);
    if (!f) {
        kprintf("Unable to open node!\n");
        return;
    }
    kprintf("Node opened, ID: %lu\n", f);
    
    kprintf("To create sub node obj1");
    err = urs_create_node(f, "obj1");
    kprintf(", err: %d\n", err);
    
    kprintf("To open new node");
    unsigned long nn = urs_open_node("/test/ramfs/obj1", 0, 0);
    if (!nn) {
        kprintf("Unable to open node!\n");
        return;
    }
    kprintf("Node opened, ID: %lu\n", nn);
    
    kprintf("To read something from our new node, ");
    memzero(buf, sizeof(buf));
    err = urs_read_node(nn, buf, sizeof(buf), &size);
    buf[127] = '\0';
    kprintf("count: %lu, str: %s\n", size, buf);
    
    kprintf("To write something, ");
    err = urs_write_node(nn, str, strlen(str) + 1, &size);
    kprintf("count: %lu\n", size);
    
    kprintf("To read something after write, ");
    memzero(buf, sizeof(buf));
    urs_seek_data(nn, 0, seek_from_begin, &size);
    err = urs_read_node(nn, buf, sizeof(buf), &size);
    buf[127] = '\0';
    kprintf("count: %lu, str: %s\n", size, buf);;
}
