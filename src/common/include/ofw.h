#ifndef __COMMON_INCLUDE_OFW__
#define __COMMON_INCLUDE_OFW__


#include "common/include/data.h"


/*
 * OFW data types
 */
#define MEMMAP_MAX_RECORDS              32
#define MAX_OFW_ARGS                    12
#define OFW_TREE_PATH_MAX_LEN           256
#define OFW_TREE_PROPERTY_MAX_NAMELEN   32
#define OFW_TREE_PROPERTY_MAX_VALUELEN  64

typedef unsigned long ofw_arg_t;
typedef unsigned long ofw_prop_t;
typedef unsigned long ofw_ihandle_t;
typedef unsigned long ofw_phandle_t;

struct ofw_args {
    ofw_arg_t service;              // Command name
    ofw_arg_t nargs;                // Number of in arguments
    ofw_arg_t nret;                 // Number of out arguments
    ofw_arg_t args[MAX_OFW_ARGS];   // List of arguments
} packedstruct;

typedef ofw_arg_t (*ofw_entry_t)(struct ofw_args *args);


/*
 * PCI node data types
 */
struct ofw_pci_reg {
    u32 space;
    u64 addr;
    u64 size;
} packedstruct;

struct ofw_pci_range {
    u32 space;
    u64 child_base;
    u64 parent_base;
    u64 size;
} packedstruct;


/*
 * Memory representation of OpenFirmware device tree node and property
 */
struct ofw_tree_prop {
    char name[OFW_TREE_PROPERTY_MAX_NAMELEN];
    int size;
    void *value;
};

struct ofw_tree_node {
    struct ofw_tree_node *parent;
    struct ofw_tree_node *peer;
    struct ofw_tree_node *child;
    
    ofw_phandle_t handle;
    char *name;
    
    int num_props;
    struct ofw_tree_prop *prop;
};


#endif
