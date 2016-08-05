#ifndef __SYSTEM_INCLUDE_URS__
#define __SYSTEM_INCLUDE_URS__


#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/stdstruct.h"


enum urs_dispatch_type {
    udisp_none,
    udisp_func,
    udisp_msg,
};

enum urs_node_type {
    unode_none,
    unode_entity,
    unode_link,
};

struct urs_dispatch {
    enum urs_dispatch_type type;
    
    union {
        void *func;
         
        struct {
            unsigned long mbox_id;
            unsigned long msg_opcode;
            unsigned long msg_num;
        };
    };
};

struct urs_operations {
    struct urs_dispatch open, close;
    struct urs_dispatch read, write;
    struct urs_dispatch list, obtain;
    struct urs_dispatch ioctl;
};

struct urs_node {
    unsigned long id;
    char *name;
    enum urs_node_type type;
    struct urs_operations op;
    
    hash_t *entries;
};

struct urs_namespace {
    unsigned long id;
    char *name;
    struct urs_node root;
};


#endif
