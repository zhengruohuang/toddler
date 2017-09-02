#include "common/include/data.h"
#include "common/include/ofw.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/print.h"
#include "hal/include/periph.h"


static struct ofw_tree_node *root;


/*
 * Node
 */
struct ofw_tree_node *ofw_node_get_child(struct ofw_tree_node *node)
{
    if (!node) {
        node = root;
    }
    
    return node->child;
}

struct ofw_tree_node *ofw_node_get_peer(struct ofw_tree_node *node)
{
    if (!node) {
        return NULL;
    } else {
        return node->peer;
    }
}

struct ofw_tree_node *ofw_node_find_by_name(struct ofw_tree_node *node, char *name)
{
    struct ofw_tree_node *ret = NULL;
    if (!node) {
        node = root;
    }
    
    // Check current node
    if (!strcmp(node->name, name)) {
        return node;
    }
    
    // Go through all child nodes
    struct ofw_tree_node *child = node->child;
    while (child) {
        ret = ofw_node_find_by_name(child, name);
        if (ret) {
            return ret;
        }
        
        child = child->peer;
    }
    
    // Not found
    return NULL;
}


/*
 * Property
 */
struct ofw_tree_prop *ofw_prop_find(struct ofw_tree_node *node, char *name)
{
    int i;
    
    if (!node) {
        node = root;
    }
    
    for (i = 0; i < node->num_props; i++) {
        struct ofw_tree_prop *prop = &node->prop[i];
        if (!strcmp(prop->name, name)) {
            return prop;
        }
    }
    
    return NULL;
}


/*
 * Initialization
 */
void init_ofw()
{
    struct boot_parameters *bp = get_bootparam();
    root = (void *)bp->ofw_tree_addr;
    kprintf("OFW device tree @ %p\n", root);
}
