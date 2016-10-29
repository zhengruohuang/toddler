/*
 * Core image FS
 */

#include "common/include/data.h"
#include "common/include/coreimg.h"
#include "kernel/include/hal.h"
#include "kernel/include/ds.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"
#include "kernel/include/urs.h"
#include "kernel/include/coreimg.h"


#define NODEID_ROOT     1
#define NODEID_BASE     10

struct open_record {
    int is_root;
    
    void *data;
    ulong size;
    int pos;
    
    ulong ref_count;
};


static int open_record_salloc_id = 0;
static hashtable_t *open_table = NULL;


/*
/* FS operations
 */
static struct open_record *get_node_by_id(ulong id)
{
    return (struct open_record *)id;
}

static int lookup(unsigned long super_id, unsigned long node_id, const char *name, int *is_link,
                  unsigned long *next_id, void *buf, unsigned long count, unsigned long *actual)
{
    int cur_id = (int)node_id;
    int index = -1;
    int ret = 0;
    
    kprintf("Lookup received, super_id: %p, node_id: %p, name: %s\n", super_id, node_id, name);
    
    switch (cur_id) {
        case 0:
            index = NODEID_ROOT;
            break;
        case NODEID_ROOT:
            index = get_core_file_index(name);
            if (-1 == index) {
                index = 0;
                ret = -1;
            } else {
                index += NODEID_BASE;
            }
            break;
        default:
            ret = -1;
            break;
    }
    
    if (*is_link) {
        *is_link = 0;
    }
    
    if (next_id) {
        *next_id = index;
    }
    
    return ret;
}

static int open(unsigned long super_id, unsigned long node_id)
{
    struct open_record *node = (struct open_record *)hashtable_obtain(open_table, node_id);
    
    if (node) {
        node->ref_count++;
        hashtable_release(open_table, node_id, node);
    }
    
    else {
        int index = (int)node_id;
        node = (struct open_record *)salloc(open_record_salloc_id);
        node->ref_count = 1;
        node->pos = 0;
        
        if (index == NODEID_ROOT) {
            node->is_root = 1;
            node->size = get_core_file_count();
        } else {
            index -= NODEID_BASE;
            node->is_root = 0;
            node->data = get_core_file_addr_by_index(index);
            node->size = get_core_file_size(index);
        }
        
        hashtable_insert(open_table, node_id, node);
    }
    
    return 0;
}

static int release(unsigned long super_id, unsigned long node_id)
{
    int ref = 0;
    struct open_record *node = (struct open_record *)hashtable_obtain(open_table, node_id);
    assert(node);
    
    ref = --node->ref_count;
    hashtable_release(open_table, node_id, node);
    
    if (!ref) {
        hashtable_remove(open_table, node_id);
        sfree(node);
    }
    
    return 0;
}

static int read(unsigned long super_id, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    ulong len = 0;
    
    u8 *src = NULL, *dest = NULL;
    
    struct open_record *node = (struct open_record *)hashtable_obtain(open_table, node_id);
    assert(node);
    
    if (!node->is_root && buf && count) {
        int index = 0;
        src = (u8 *)node->data;
        dest = (u8 *)buf;
        
        while (len < count && node->pos < node->size) {
            dest[len] = src[node->pos];
            
            len++;
            node->pos++;
        }
    }
    
    hashtable_release(open_table, node_id, node);
    
    if (actual) {
        *actual = len;
    }
    
    return 0;
}

static ulong seek_node(struct open_record *node, unsigned long offset, enum urs_seek_from from)
{
    int off = (int)offset;
    
    switch (from) {
    case seek_from_begin:
        if (off <= node->size) {
            node->pos = off;
        }
        break;
    case seek_from_cur_fwd:
        if (node->pos + off <= node->size) {
            node->pos += off;
        }
        break;
    case seek_from_cur_bwd:
        if (node->pos - off >= 0) {
            node->pos -= off;
        }
        break;
    case seek_from_end:
        if (node->size - node->pos >= 0) {
            node->pos = node->size - node->pos;
        }
        break;
    default:
        break;
    }
    
    return (ulong)node->pos;
}

static int seek_data(unsigned long super_id, unsigned long node_id, unsigned long offset, enum urs_seek_from from, unsigned long *newpos)
{
    unsigned long pos = 0;
    struct open_record *node = (struct open_record *)hashtable_obtain(open_table, node_id);
    assert(node);
    
    if (!node->is_root) {
        pos = seek_node(node, offset, from);
    } else {
        pos = node->pos;
    }
    
    hashtable_release(open_table, node_id, node);
    
    if (*newpos) {
        *newpos = pos;
    }
    
    return 0;
}

static int list(unsigned long super_id, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    ulong len = 0;
    char name[64];
    int ret = 0;
    
    struct open_record *node = (struct open_record *)hashtable_obtain(open_table, node_id);
    assert(node);
    
    if (node->is_root) {
        if (node->pos >= node->size) {
            ret = -1;
        } else {
            if (buf && count) {
                get_core_file_name(node->pos, name, sizeof(name));
                len = strlen(name) + 1;
//                 kprintf("found file: %s\n", name);
            }
            
            node->pos++;
        }
    }
    
    hashtable_release(open_table, node_id, node);
    
    if (len > count) {
        len = count;
    }
    memcpy(name, buf, len);
    
    if (actual) {
        *actual = len;
    }
    
    return ret;
}

static int seek_list(unsigned long super_id, unsigned long node_id, unsigned long offset, enum urs_seek_from from, unsigned long *newpos)
{
    unsigned long pos = 0;
    struct open_record *node = (struct open_record *)hashtable_obtain(open_table, node_id);
    assert(node);
    
    if (node->is_root) {
        pos = seek_node(node, offset, from);
    } else {
        pos = node->pos;
    }
    
    hashtable_release(open_table, node_id, node);
    
    if (*newpos) {
        *newpos = pos;
    }
    
    return 0;
}


/*
 * Init
 */
#define REG_OP_FUNC(type, func) urs_register_op(super_id, type, func, 0, 0, 0)

void init_coreimgfs()
{
    unsigned long super_id = urs_register("coreimg://");
    assert(super_id);
    
    // Register operations
    REG_OP_FUNC(uop_lookup, lookup);
    REG_OP_FUNC(uop_open, open);
    REG_OP_FUNC(uop_release, release);
    
    REG_OP_FUNC(uop_read, read);
    REG_OP_FUNC(uop_seek_data, seek_data);
    
    REG_OP_FUNC(uop_list, list);
    REG_OP_FUNC(uop_seek_list, seek_list);
    
    open_record_salloc_id = salloc_create(sizeof(struct open_record), 0, 0, NULL, NULL);
    open_table = hashtable_new(0, NULL, NULL);
    
    kprintf("Coreimg FS initialized, super id: %p, record salloc ID: %d\n", super_id, open_record_salloc_id);
}
