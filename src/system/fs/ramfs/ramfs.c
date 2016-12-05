#include "common/include/data.h"
#include "common/include/urs.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/assert.h"
#include "klibc/include/sys.h"
#include "klibc/include/time.h"
#include "klibc/include/kthread.h"


#define RAMFS_BLOCK_SIZE    128


struct ramfs_block {
    struct ramfs_block *prev;
    struct ramfs_block *next;
    
    u8 data[RAMFS_BLOCK_SIZE];
};

struct ramfs_data {
    unsigned long block_count;
    unsigned long size;
    
    struct ramfs_block *head;
    struct ramfs_block *tail;
};

struct ramfs_sub {
    unsigned long count;
    hash_t *entries;
};

struct ramfs_node {
    unsigned long id;
    char *name;
    
    unsigned long ref_count;
    unsigned long open_count;
    
    time_t create_time;
    time_t read_time;
    time_t write_time;
    time_t list_time;
    time_t change_time;
    
    struct ramfs_node *parent;
    struct ramfs_data data;
    struct ramfs_sub sub;
    char *link;
};

struct ramfs_open {
    unsigned long id;
    struct ramfs_node *node;
    
    unsigned long data_pos;
    unsigned long sub_pos;
};


static unsigned long block_salloc_id;
static unsigned long node_salloc_id;
static unsigned long open_salloc_id;
static hash_t *ramfs_table;


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
static int seek_data_block(struct ramfs_data *data, unsigned long pos, unsigned long *real_pos, struct ramfs_block **cur_block, unsigned long *cur_offset)
{
    struct ramfs_block *block = data->head;
    unsigned long offset = 0;
    
    if (pos > data->size) {
        pos = data->size;
    }
    
    offset = pos;
    while (offset >= RAMFS_BLOCK_SIZE) {
        offset -= RAMFS_BLOCK_SIZE;
        assert(block);
        block = block->next;
    }
    
    if (real_pos) {
        *real_pos = pos;
    }
    if (cur_block) {
        *cur_block = block;
    }
    if (cur_offset) {
        *cur_offset = offset;
    }
    
    return 0;
}

static unsigned long read_data_block(struct ramfs_data *data, unsigned long pos, u8 *buf, unsigned long count)
{
    int index = 0;
    unsigned long cur_pos = pos;
    
    struct ramfs_block *block;
    unsigned long offset = 0;
    if (seek_data_block(data, pos, &cur_pos, &block, &offset)) {
        return 0;
    }
    
    while (cur_pos < data->size && (unsigned long)index < count && block) {
        // Copy data
        buf[index] = block->data[offset];
        index++;
        offset++;
        cur_pos++;
        
        // Move to next data block
        if (offset == RAMFS_BLOCK_SIZE) {
            block = block->next;
            offset = 0;
        }
    }
    
//     kprintf("data read: %s, size: %p\n", (char *)buf, index);
    return index;
}

static unsigned long write_data_block(struct ramfs_data *data, unsigned long pos, u8 *buf, unsigned long count)
{
    int index = 0;
    unsigned long cur_pos = pos;
    
    struct ramfs_block *block;
    unsigned long offset = 0;
    if (seek_data_block(data, pos, &cur_pos, &block, &offset)) {
        return 0;
    }
    
//     kprintf("write, buf: %s, count: %p\n", buf, count);
    
    while ((unsigned long)index < count) {
        // Allocate a new block if necessary
        if (!block) {
            block = (struct ramfs_block *)salloc(block_salloc_id);
            offset = 0;
            
            block->next = NULL;
            block->prev = data->tail;
            
            if (data->tail) {
                data->tail->next = block;
            }
            
            data->tail = block;
            if (!data->head) {
                data->head = block;
            }
            
            data->block_count++;
        }
        
        // Copy data
        block->data[offset] = buf[index];
        index++;
        offset++;
        cur_pos++;
        data->size++;
        
        // Move to next data block
        if (offset == RAMFS_BLOCK_SIZE) {
            block = block->next;
            offset = 0;
        }
    }
    
    return index;
}

static int truncate_data_block(struct ramfs_data *data, unsigned long pos)
{
    unsigned long cur_pos = pos;
    struct ramfs_block *block;
    unsigned long offset = 0;
    if (seek_data_block(data, pos, &cur_pos, &block, &offset)) {
        return 0;
    }
    
    // Mark cur block as the last block
    data->tail = block;
    block->next = NULL;
    
    // Free following blocks
    block = block->next;
    while (block) {
        struct ramfs_block *next = block->next;
        sfree(block);
        data->block_count--;
        block = next;
    }
    
    // Set file size
    data->size = cur_pos;
    
    return 0;
}

static void free_data_block(struct ramfs_data *data)
{
    // Free all blocks
    struct ramfs_block *block = data->head;
    while (block) {
        struct ramfs_block *next = block->next;
        sfree(block);
        block = next;
    }
    
    // Reset data
    data->block_count = 0;
    data->head = data->tail = NULL;
    data->size = 0;
}


/*
 * Helpers
 */
static struct ramfs_node *get_node_by_id(unsigned long super_id, unsigned long node_id)
{
    return (struct ramfs_node *)node_id;
}

static struct ramfs_node *create_node(const char *name, struct ramfs_node *parent)
{
    struct ramfs_node *node = (struct ramfs_node *)salloc(node_salloc_id);
    
    node->id = (unsigned long)node;
    node->name = strdup(name);
    node->ref_count = 0;
    
    node->parent = parent;
    
    node->data.block_count = 0;
    node->data.size = 0;
    node->data.head = NULL;
    node->data.tail = NULL;
    
    node->sub.count = 0;
    node->sub.entries = NULL;
    
    node->link = NULL;
    
    node->create_time = time();
    node->read_time = 0;
    node->write_time = 0;
    node->change_time = 0;
    
    return node;
}

static struct ramfs_open *get_open_by_id(unsigned long open_id)
{
    return (struct ramfs_open *)open_id;
}

static struct ramfs_open *create_open(unsigned long super_id, unsigned long node_id)
{
    struct ramfs_node *node = get_node_by_id(super_id, node_id);
    if (!node) {
        return NULL;
    }
    
    struct ramfs_open *open = (struct ramfs_open *)salloc(open_salloc_id);
    
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
    struct ramfs_node *node = NULL, *next = NULL;
    
    kprintf("Lookup received, super_id: %p, node_id: %lu, name: %s\n", super_id, node_id, name);
    
    // Find out the next node
    if (node_id) {
        node = get_node_by_id(super_id, node_id);
        assert(node);
        
        node->ref_count--;
        if (!node->sub.count) {
            return 0;
        }
        
        next = (struct ramfs_node *)hash_obtain(node->sub.entries, (void *)name);
        if (!next) {
            hash_release(node->sub.entries, (void *)name, next);
            return 0;
        }
    } else {
        node = (struct ramfs_node *)hash_obtain(ramfs_table, (void *)super_id);
        hash_release(ramfs_table, (void *)super_id, node);
        next = node;
        assert(next);
    }
    
    // Sym link
    if (next->link) {
        unsigned long cpy_index = 0;
        unsigned long link_len = strlen(next->link) + 1;
        u8 *cpy = (u8 *)buf;
        
        if (is_link) {
            *is_link = 1;
        }
        
        if (buf) {
            while (cpy_index < count && cpy_index < link_len) {
                cpy[cpy_index] = (u8)next->link[cpy_index];
                cpy_index++;
            }
        }
        
        if (actual) {
            *actual = cpy_index;
        }
    }
    
    // Regular node
    else {
        next->ref_count++;
        
        if (is_link) {
            *is_link = 0;
        }
        
        if (next_id) {
            *next_id = next->id;
        }
    }
    
    if (node_id) {
        hash_release(node->sub.entries, (void *)name, next);
    }
    
    return 0;
}

static int open(unsigned long super_id, unsigned long node_id, unsigned long *open_id)
{
    struct ramfs_node *node = get_node_by_id(super_id, node_id);
    if (!node) {
        return ENOENT;
    }
    
    struct ramfs_open *open = create_open(super_id, node_id);
    if (!open) {
        return EBADF;
    }
    
    if (open_id) {
        *open_id = open->id;
    }
    
    node->change_time = time();
    
    return EOK;
}

static int close(unsigned long open_id)
{
    struct ramfs_open *open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    open->node->change_time = time();
    
    // FIXME: Lock needed
    open->node->open_count--;
    sfree(open);
    
    return EOK;
}

static int read(unsigned long open_id, void *buf, unsigned long count, unsigned long *actual)
{
    unsigned long result = 0;
    
    struct ramfs_open *open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    result = read_data_block(&open->node->data, (unsigned long)open->data_pos, buf, count);
    open->data_pos = result;
    
    if (actual) {
        *actual = result;
    }
    
    open->node->read_time = time();
    
    return 0;
}

static int write(unsigned long open_id, void *buf, unsigned long count, unsigned long *actual)
{
    unsigned long result = 0;
    
    struct ramfs_open *open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    result = write_data_block(&open->node->data, (unsigned long)open->data_pos, buf, count);
    open->data_pos = result;
    
    if (actual) {
        *actual = result;
    }
    
    open->node->write_time = time();
    
    return 0;
}

static int truncate(unsigned long open_id)
{
    int result = 0;
    
    struct ramfs_open *open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    result = truncate_data_block(&open->node->data, (unsigned long)open->data_pos);
    
    open->node->write_time = time();
    
    return result;
}

static int seek_data(unsigned long open_id, u64 offset, enum urs_seek_from from, u64 *newpos)
{
    unsigned long pos = 0;
    
    struct ramfs_open *open = NULL;
    struct ramfs_node *node = NULL;
    
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

static int list(unsigned long open_id, void *buf, unsigned long count, unsigned long *actual)
{
    struct ramfs_node *sub = NULL;
    unsigned long len = 0;
    int result = 0;
     
    struct ramfs_open *open = NULL;
    struct ramfs_node *node = NULL;
    
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
//     kprintf("to obtain at pos: %p, entries: %p, count: %p\n", node->sub.pos, node->sub.entries, node->sub.count);
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

static int seek_list(unsigned long open_id, u64 offset, enum urs_seek_from from, u64 *newpos)
{
    unsigned long pos = 0;
    
    struct ramfs_open *open = NULL;
    struct ramfs_node *node = NULL;
    
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

static int create(unsigned long open_id, char *name, enum urs_create_type type, unsigned int flags, char *target, unsigned long target_open_id)
{
    struct ramfs_node *sub = NULL;
    
    struct ramfs_open *open = NULL;
    struct ramfs_node *node = NULL;
    
    open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    node = open->node;
    if (!node) {
        return ECLOSED;
    }
    
    switch (type) {
    case ucreate_node:
        sub = create_node(name, node);
        break;
    case ucreate_sym_link:
        sub = create_node(name, node);
        sub->link = strdup(target);
        break;
    case ucreate_hard_link: {
        struct ramfs_open *target_open = get_open_by_id(target_open_id);
        if (!target_open) {
            return EBADF;
        }
        sub = open->node;
        assert(sub);
        sub->ref_count++;
        break;
    }
    case ucreate_dyn_link:
        return -1;
    case ucreate_none:
        return 0;
    default:
        break;
    }
    
    if (sub) {
        if (!node->sub.entries) {
            node->sub.entries = hash_new(0, urs_hash_func, urs_hash_cmp);
            kprintf("New has table created @ %p\n", node->sub.entries);
        }
        
        kprintf("To insert into hash table, name: %s, sub: %p\n", name, sub);
        if (hash_insert(node->sub.entries, sub->name, sub)) {
            if (sub->link) {
                free(sub->link);
            }
            
            sfree(sub);
            return -2;
        };
        
        node->sub.count++;
    }
    
    return 0;
}

static int remove(unsigned long open_id, int erase)
{
    struct ramfs_node *parent = NULL;
    struct ramfs_open *open = NULL;
    struct ramfs_node *node = NULL;
    
    open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    node = open->node;
    if (!node) {
        return ECLOSED;
    }
    
    kprintf("To remove node, ref count: %p\n", node->ref_count);
    
    // Get the parent and make sure this is not root
    parent = node->parent;
    if (!parent) {
        return -2;
    }
    
    // Make sure this node does not have any sub entries
    if (node->sub.count) {
        return -1;
    }
    
    // Remove the node from parent
    assert(parent->sub.entries);
    assert(parent->sub.count);
    
    hash_remove(parent->sub.entries, node->name);
    parent->sub.count--;
    
    // Free this node
    node->ref_count--;
    
    if (!node->ref_count) {
        if (node->sub.entries) {
            sfree(node->sub.entries);
        }
        if (node->link) {
            free(node->link);
        }
        free_data_block(&node->data);
        free(node->name);
    }
    
    // Emptify open
    open->node = NULL;
    open->data_pos = open->sub_pos = 0;
    
    return EOK;
}

static int rename(unsigned long open_id, char *name)
{
    struct ramfs_node *parent = NULL;
    
    struct ramfs_open *open = NULL;
    struct ramfs_node *node = NULL;
    
    open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    node = open->node;
    if (!node) {
        return ECLOSED;
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
    
    return EOK;
}

static int stat(unsigned long open_id, struct urs_stat *stat)
{
    struct ramfs_open *open = NULL;
    struct ramfs_node *node = NULL;
    
    open = get_open_by_id(open_id);
    if (!open) {
        return EBADF;
    }
    
    node = open->node;
    if (!node) {
        return ECLOSED;
    }
    
    if (stat) {
        struct ramfs_node *n = open->node;
        
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
 * Message handler
 */
asmlinkage void ramfs_msg_handler(msg_t *msg)
{
    int result = EOK;
    unsigned long ret_mbox_id = msg->mailbox_id;
    
    // Get common data fields
    // Note that we also discard the 0th field of the msg as it is the same as opcode
    enum urs_op_type op = (enum urs_op_type)msg->opcode;
    
    // Setup the reply msg
    msg_t *r = NULL;
    
    switch (op) {
    // Look up
    case uop_lookup: {
        unsigned long super_id = msg->params[1].value;
        unsigned long node_id = msg->params[2].value;
        char *name = (char *)((unsigned long)msg + msg->params[3].offset);
        int is_link = 0;
        unsigned long next_id = 0;
        
        result = lookup(super_id, node_id, name, &is_link, &next_id, NULL, 0, NULL);
        
        kprintf("name: %s, node: %p, is link: %d, next: %p\n", name, node_id, is_link, next_id);
        
        r = syscall_msg();
        msg_param_value(r, (unsigned long)is_link);
        msg_param_value(r, next_id);
        msg_param_buffer(r, NULL, 0);
        msg_param_value(r, 0);
        
        break;
    }
    
    case uop_open: {
        unsigned long super_id = msg->params[1].value;
        unsigned long node_id = msg->params[2].value;
        unsigned long open_id = 0;
        
        result = open(super_id, node_id, &open_id);
        
        r = syscall_msg();
        msg_param_value(r, open_id);
        
        break;
    }
    
    case uop_release: {
        unsigned long open_id = msg->params[2].value;
        result = close(open_id);
        break;
    }
    
    // Data
    case uop_read: {
        unsigned long open_id = msg->params[2].value;
        u8 buf[64];
        unsigned long count = sizeof(buf);
        unsigned long actual = 0;
        if (msg->params[3].value < count) {
            count = msg->params[3].value;
        }
        
        result = read(open_id, buf, count, &actual);
        
        r = syscall_msg();
        msg_param_buffer(r, buf, actual);
        msg_param_value(r, actual);
        
        break;
    }
    
    case uop_write: {
        unsigned long open_id = msg->params[2].value;
        void *buf = (void *)((unsigned long)msg + msg->params[3].offset);
        unsigned long count = msg->params[4].value;
        unsigned long actual = 0;
        
        result = write(open_id, buf, count, &actual);
        
        r = syscall_msg();
        msg_param_value(r, actual);
        
        break;
    }
    
    case uop_truncate: {
        unsigned long open_id = msg->params[2].value;
        result = truncate(open_id);
        break;
    }
    
    case uop_seek_data: {
        unsigned long open_id = msg->params[2].value;
        u64 offset = msg->params[3].value64;
        enum urs_seek_from from = (enum urs_seek_from)msg->params[4].value;
        u64 newpos = 0;
        
        result = seek_data(open_id, offset, from, &newpos);
        
        r = syscall_msg();
        msg_param_value64(r, newpos);
        
        break;
    }
    
    // List
    case uop_list: {
        unsigned long open_id = msg->params[2].value;
        u8 buf[64];
        unsigned long count = sizeof(buf);
        unsigned long actual = 0;
        
        if (msg->params[3].value < count) {
            count = msg->params[3].value;
        }
        
        result = list(open_id, buf, count, &actual);
        
        r = syscall_msg();
        msg_param_buffer(r, buf, actual);
        msg_param_value(r, actual);
        
        break;
    }
    
    case uop_seek_list: {
        unsigned long open_id = msg->params[2].value;
        u64 offset = msg->params[3].value64;
        enum urs_seek_from from = (enum urs_seek_from)msg->params[4].value;
        u64 newpos = 0;
        
        result = seek_list(open_id, offset, from, &newpos);
        
        r = syscall_msg();
        msg_param_value64(r, newpos);
        
        break;
    }
    
    // Create
    case uop_create: {
        unsigned long open_id = msg->params[2].value;
        char *name = (void *)((unsigned long)msg + msg->params[3].offset);
        enum urs_create_type type = (enum urs_create_type)msg->params[4].value;
        unsigned int flags = (unsigned int)msg->params[5].value;
        char *target = (void *)((unsigned long)msg + msg->params[6].offset);
        unsigned long target_open_id = msg->params[7].value;
        
        result = create(open_id, name, type, flags, target, target_open_id);
        break;
    }
    
    case uop_remove: {
        unsigned long open_id = msg->params[2].value;
        int erase = (int)msg->params[3].value;
        result = remove(open_id, erase);
        break;
    }
    
    case uop_rename: {
        unsigned long open_id = msg->params[2].value;
        char *name = (void *)((unsigned long)msg + msg->params[3].offset);
        result = rename(open_id, name);
        break;
    }
    
    // Stat
    case uop_stat: {
        unsigned long open_id = msg->params[2].value;
        struct urs_stat s;
        
        result = stat(open_id, &s);
        
        r = syscall_msg();
        msg_param_buffer(r, &s, sizeof(struct urs_stat));
        
        break;
    }
    
    default:
        break;
    }
    
    // Return value
    if (!r) {
        r = syscall_msg();
    }
    r->mailbox_id = ret_mbox_id;
    msg_param_value(r, (unsigned long)result);
    
//     kprintf("To call syscall respond!\n");
    
    // Issue the KAPI and obtain the result
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}


/*
 * Super
 */
#define REG_OP(op, handler) do {                                        \
    unsigned long func_num = alloc_msg_num();                           \
    ops.entries[op].type = ureg_msg;                                    \
    ops.entries[op].msg_opcode = (unsigned long)op;                     \
    ops.entries[op].msg_func_num = func_num;                            \
    syscall_reg_msg_handler(func_num, ramfs_msg_handler);               \
} while (0)

int register_ramfs(char *path)
{
    struct urs_reg_ops ops;
    struct ramfs_node *root = NULL;
    unsigned long super_id = 0;
    
    // Prepare operations
    REG_OP(uop_lookup, lookup);
    REG_OP(uop_open, open);
    REG_OP(uop_release, release);
    
    REG_OP(uop_read, read);
    REG_OP(uop_write, write);
    REG_OP(uop_truncate, truncate);
    REG_OP(uop_seek_data, seek_data);
    
    REG_OP(uop_list, list);
    REG_OP(uop_seek_list, seek_list);
    
    REG_OP(uop_create, create);
    REG_OP(uop_remove, remove);
    REG_OP(uop_rename, rename);
    
    REG_OP(uop_stat, stat);
    
    // Register the FS
    super_id = kapi_urs_reg_super(path, "ramfs", 0, &ops);
    if (!super_id) {
        return -2;
    }
    
    root = create_node("/", NULL);
    if (!root) {
        return -3;
    }
    
    hash_insert(ramfs_table, (void *)super_id, root);
    
    return 0;
}


/*
 * Init RAM FS
 */
void init_ramfs()
{
    ramfs_table = hash_new(0, NULL, NULL);
    open_salloc_id = salloc_create(sizeof(struct ramfs_open), 0, NULL, NULL);
    node_salloc_id = salloc_create(sizeof(struct ramfs_node), 0, NULL, NULL);
    block_salloc_id = salloc_create(sizeof(struct ramfs_block), 0, NULL, NULL);
    
    register_ramfs("ramfs://");
}


/*
 * Tests
 */
void test_ramfs()
{
//     int err = 0;
//     unsigned long size = 0;
//     char *str = "this is a test string!";
//     char buf[128];
//     
//     kprintf("To register RAM FS @ /test/ramfs\n");
//     register_ramfs("/test/ramfs");
//     kprintf("Registered!");
//     
//     kprintf("To open URS node");
//     unsigned long f = urs_open_node("/test/ramfs", 0, 0);
//     if (!f) {
//         kprintf("Unable to open node!\n");
//         return;
//     }
//     kprintf("Node opened, ID: %lu\n", f);
//     
//     kprintf("To create sub node obj1");
//     err = urs_create_node(f, "obj1");
//     kprintf(", err: %d\n", err);
//     
//     kprintf("To open new node");
//     unsigned long nn = urs_open_node("/test/ramfs/obj1", 0, 0);
//     if (!nn) {
//         kprintf("Unable to open node!\n");
//         return;
//     }
//     kprintf("Node opened, ID: %lu\n", nn);
//     
//     kprintf("To read something from our new node, ");
//     memzero(buf, sizeof(buf));
//     err = urs_read_node(nn, buf, sizeof(buf), &size);
//     buf[127] = '\0';
//     kprintf("count: %lu, str: %s\n", size, buf);
//     
//     kprintf("To write something, ");
//     err = urs_write_node(nn, str, strlen(str) + 1, &size);
//     kprintf("count: %lu\n", size);
//     
//     kprintf("To read something after write, ");
//     memzero(buf, sizeof(buf));
//     urs_seek_data(nn, 0, seek_from_begin, &size);
//     err = urs_read_node(nn, buf, sizeof(buf), &size);
//     buf[127] = '\0';
//     kprintf("count: %lu, str: %s\n", size, buf);
}
