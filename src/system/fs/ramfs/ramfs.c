#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"
#include "system/include/urs.h"


#define RAMFS_BLOCK_SIZE    512


struct ramfs_block {
    struct ramfs_block *prev;
    struct ramfs_block *next;
    unsigned char data[RAMFS_BLOCK_SIZE];
};

struct ramfs_data {
    unsigned long block_count;
    unsigned long size;
    unsigned long pos;
    
    struct ramfs_block *head;
    struct ramfs_block *tail;
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


static unsigned long node_salloc_id;
static hash_t *ramfs_table;


/*
 * Init RAM FS
 */
void init_ramfs()
{
    ramfs_table = hash_new(0, NULL, NULL);
    node_salloc_id = salloc_create(sizeof(struct ramfs_node), 0, NULL, NULL);
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
    
    node->sub.count = 0;
    node->sub.pos = 0;
    node->sub.entries = NULL;
    
    return node;
}

static int lookup(unsigned long super_id, unsigned long node_id, const char *name, unsigned long *next_id)
{
    struct urs_super *super = get_super_by_id(super_id);
    struct ramfs_node *node = NULL, *next = NULL;
    
    if (next_id) {
        *next_id = 0;
    }
    
    if (node_id) {
        node = get_node_by_id(node_id);
    } else {
        node = (struct ramfs_node *)hash_obtain(ramfs_table, super_id);
        hash_release(ramfs_table, super_id, node);
    }
    
    if (!node->sub.count) {
        return 0;
    }
    
    next = (struct ramfs_node *)hash_obtain(node->sub.entries, name);
    if (!next) {
        return 0;
    }
    
    next->ref_count++;
    if (next_id) {
        *next_id = next->id;
    }
    
    hash_release(node->sub.entries, name, next);
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
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
}

static int write(unsigned long super_id, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    struct ramfs_node *node = get_node_by_id(node_id);
    if (!node) {
        return -1;
    }
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
        return 0;
    }
    
    sfree(sub);
    return -2;
}

static int remove(unsigned long super_id, unsigned long node_id)
{
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
int register_ramfs(char *path)
{
    struct ramfs_node *root = NULL;
    unsigned long super_id = 0;
    
    struct urs_super *super = register_super(path);
    if (!super) {
        return -1;
    }
    
    super_id = super->id;
    if (!super_id) {
        return -2;
    }
    
    root = create_node("/", NULL);
    if (!root) {
        return -3;
    }
    
    hash_insert(ramfs_table, super->id, root);
    return 0;
}

int unregister_ramfs(char *path)
{
}
