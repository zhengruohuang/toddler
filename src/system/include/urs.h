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

enum urs_op_type {
    uop_none,
    uop_open,
    uop_close,
    uop_read,
    uop_write,
    uop_list,
    uop_obtain,
    uop_ioctl,
    uop_count,
};

struct urs_dispatch {
    enum urs_dispatch_type type;
    
    union {
        void *func;
         
        struct {
            unsigned long mbox_id;
            unsigned long msg_opcode;
            unsigned long msg_func_num;
        };
    };
};

struct urs_node {
    unsigned long id;
    char *name;
    enum urs_node_type type;
    struct urs_dispatch ops[uop_count];
    
    hash_t *entries;
};

struct urs_namespace {
    unsigned long id;
    char *name;
    struct urs_node *root;
};


extern int parse_url_namespace(char *path, char **out);
extern int parse_url_node(char *path, int start, char **out);

#endif
