#ifndef __SYSTEM_INCLUDE_URS__
#define __SYSTEM_INCLUDE_URS__


#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/stdstruct.h"


enum urs_disp_type {
    udisp_none,
    udisp_link,
    udisp_func,
    udisp_msg,
};

enum urs_op_type {
    uop_none,
    
    uop_lookup,
    uop_release,
    
    uop_read,
    uop_write,
    uop_list,
    
    uop_create,
    uop_remove,
    uop_rename,
    
    uop_stat,
    uop_ioctl,
    
    uop_count,
};

struct urs_disp {
    enum urs_disp_type type;
    
    union {
        char *link;
        int (*func)(unsigned long dispatch_id, ...);
        struct {
            unsigned long mbox_id;
            unsigned long msg_opcode;
            unsigned long msg_func_num;
        };
    };
};

struct urs_super {
    unsigned long id;
    char *path;
    unsigned long ref_count;
    
    struct urs_disp ops[uop_count];
};

struct urs_node {
    unsigned long id;
    unsigned long ref_count;
    
    unsigned long dispatch_id;
    struct urs_super *super;
};

struct urs_open {
    unsigned long id;
    char *path;
    unsigned long ref_count;
    
    unsigned long long cur_pos;
    unsigned long long size;
    
    struct urs_node *node;
};


extern void init_urs();

extern struct urs_super *register_super(char *path);
extern int register_super_op(
    unsigned long id, enum urs_op_type op, void *func, char *link,
    unsigned long mbox_id, unsigned long msg_opcode, unsigned long msg_func_num
);

struct urs_open *open_node(char *path, unsigned int mode, unsigned long process_id);
int close_node(unsigned long id, unsigned long process_id);
unsigned long read_node(unsigned long id, void *buf, unsigned long count);
unsigned long write_node(unsigned long id, void *buf, unsigned long count);
int list_node(unsigned long id, void *buf, unsigned long count);
int create_node(unsigned long id, char *name);
int remove_node(unsigned long id);
int rename_node(unsigned long id, char *name);


#endif
