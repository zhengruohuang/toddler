/*
 * Universal Resource System - URS
 */

#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "system/include/urs.h"


static int namespace_salloc_id;
static int node_salloc_id;

static hash_t *namespaces;


/*
 * Hash table
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
 * Init URS
 */
void init_urs()
{
    namespaces = hash_new(0, urs_hash_func, urs_hash_cmp);
    namespace_salloc_id = salloc_create(sizeof(struct urs_namespace), 0, NULL, NULL);
    
    node_salloc_id = salloc_create(sizeof(struct urs_node), 0, NULL, NULL);
}


/*
 * Find node
 */
struct urs_node *find_node(char *path)
{
    char *name = NULL;
    int start = 0;
    struct urs_namespace *ns = NULL;
    struct urs_node *node = NULL;
    hash_t *entries;
    
    // Find out the namespace
    start = parse_url_namespace(path, &name);
    if (-1 == start) {
        return NULL;
    }
    
    ns = (struct urs_namespace *)hash_obtain(namespaces, name);
    if (!ns) {
        return NULL;
    }
    
    node = ns->root;
    hash_release(namespaces, name, ns);
    
    // Start getting node
    free(name);
    start = parse_url_node(path, start, &name);
    
    while (start != -1) {
        entries = node->entries;
        
        node = (struct urs_node *)hash_obtain(entries, name);
        if (!node) {
            return NULL;
        }
        hash_release(entries, name, node);
        
        // Get the next node
        start = parse_url_node(path, start, &name);
    }
    
    return node;
}

struct urs_node *find_node_by_id(unsigned long urs_node_id)
{
    return (struct urs_node *)urs_node_id;
}


/*
 * Create namespace and node
 */
static void init_urs_node(struct urs_node *node)
{
    int i;
    
    node->entries = NULL;
    node->id = (unsigned long)node;
    node->type = unode_none;
    node->name = ".";
    for (i = 0; i < uop_count; i++) {
        node->ops[i].type = udisp_none;
    }
}

struct urs_namespace *create_namespace(char *name)
{
    struct urs_namespace *ns = NULL;
    
    if (hash_contains(namespaces, name)) {
        return NULL;
    }
    
    ns = (struct urs_namespace *)salloc(namespace_salloc_id);
    if (!ns) {
        return NULL;
    }
    
    ns->id = (unsigned long)ns;
    ns->name = strdup(name);
    
    ns->root = (struct urs_node *)salloc(node_salloc_id);
    if (!ns->root) {
        sfree(ns);
        return NULL;
    }
    
    init_urs_node(ns->root);
    ns->root->name = "/";
    
    return ns;
}

struct urs_node *create_node(char *path, char *name)
{
    struct urs_node *node = NULL;
    struct urs_node *parent = find_node(path);
    
    if (!parent) {
        return NULL;
    }
    
    node = (struct urs_node *)salloc(node_salloc_id);
    if (!node) {
        return NULL;
    }
    
    init_urs_node(node);
    node->name = strdup(name);
    
    return node;
}

int activate_node_entity(char *path)
{
    struct urs_node *node = find_node(path);
    if (!node) {
        return -1;
    }
    
    if (node->type != unode_none) {
        return -1;
    }
    
    node->type = unode_entity;
    return 0;
}


/*
 * Register operations
 */
int register_op_func(unsigned long urs_node_id, enum urs_op_type op, void *func)
{
    struct urs_node *node = find_node_by_id(urs_node_id);
    
    node->ops[op].type = udisp_func;
    node->ops[op].func = func;
    
    return 0;
}

int register_op_msg(unsigned long urs_node_id, enum urs_op_type op, unsigned long mbox_id, unsigned long opcode, unsigned long func_num)
{
    struct urs_node *node = find_node_by_id(urs_node_id);
    
    node->ops[op].type = udisp_msg;
    node->ops[op].mbox_id = mbox_id;
    node->ops[op].msg_opcode = opcode;
    node->ops[op].msg_func_num = func_num;
    
    return 0;
}
