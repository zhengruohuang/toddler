#ifndef __DRIVER_INCLUDE_DEVFS__
#define __DRIVER_INCLUDE_DEVFS__


#include "common/include/data.h"
#include "klibc/include/stdstruct.h"


/*
 * Device tree
 */
struct dev_ops {
    // Generic operations
    int (*init)();
    
    // Block operations
    int (*enumerate)();
    
    // Raw/Block operations
    int (*open)();
    int (*close)();
    int (*read)();
    int (*write)();
    int (*seek)();
    int (*ioctl)();
};

struct dev_desc {
    unsigned long dev_id;
    
    // Driver specific descriptor
    void *driver_dev_desc;
};

enum dev_tree_node_type {
    DEV_NODE_UNKNOWN,
    DEV_NODE_DIR,
    DEV_NODE_BUS,
    DEV_NODE_RAW,
    DEV_NODE_BLOCK,
};

struct dev_tree_node {
    int type;
    char *name;
    struct dev_ops ops;
    struct dev_desc desc;
    
    dlist_t nexts;
};

extern void init_dev_tree();
extern struct dev_tree_node *dev_tree_create(int type, char *name);
extern int dev_tree_attach(struct dev_tree_node *parent, struct dev_tree_node *node);


/*
 * Device FS
 */
struct open_desc {
    unsigned long open_id;
};

extern void init_devfs();


#endif
