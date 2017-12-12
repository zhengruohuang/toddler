#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/assert.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "driver/include/devfs.h"


static unsigned long node_salloc_id = -1;
static volatile struct dev_tree_node *root;

void init_dev_tree()
{
    node_salloc_id = salloc_create(sizeof(struct dev_tree_node), 0, NULL, NULL);
}

struct dev_tree_node *dev_tree_create(int type, char *name)
{
    struct dev_tree_node *node = (struct dev_tree_node *)salloc(node_salloc_id);
    node->type = type;
    node->name = strdup(name);
    
    return node;
}


int dev_tree_attach(struct dev_tree_node *parent, struct dev_tree_node *node)
{
    // Add the new node to the tree
    if (!parent) {
        assert(!root);
        root = node;
    }
    
    else {
        dlist_push_back(&parent->nexts, node);
    }
    
    return EOK;
}
