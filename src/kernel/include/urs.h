#ifndef __KERNEL_INCLUDE_URS__
#define __KERNEL_INCLUDE_URS__


#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/urs.h"
#include "kernel/include/ds.h"


enum urs_disp_type {
    udisp_none,
    udisp_func,
    udisp_msg,
};

struct urs_disp {
    enum urs_disp_type type;
    
    union {
        int (*func)(unsigned long super_id, unsigned long dispatch_id, ...);
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
    char *name;
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
    
    u64 data_pos;
    u64 data_size;
    
    u64 list_pos;
    u64 list_size;
    
    struct urs_node *node;
};


extern void init_urs();

extern unsigned long urs_register(char *path, char *name, unsigned int flags, struct urs_reg_ops *ops);
extern int urs_unregister(char *path);
// extern int urs_register_op(
//     unsigned long id, enum urs_op_type op, void *func,
//     unsigned long mbox_id, unsigned long msg_opcode, unsigned long msg_func_num
// );

extern unsigned long urs_open_node(char *path, unsigned int flags, unsigned long process_id);
extern int urs_close_node(unsigned long id);

extern int urs_read_node(unsigned long id, void *buf, unsigned long count, unsigned long *actual);
extern int urs_write_node(unsigned long id, void *buf, unsigned long count, unsigned long *actual);
extern int urs_truncate_node(unsigned long id);
extern int urs_seek_data(unsigned long id, unsigned long offset, enum urs_seek_from from, unsigned long *newpos);

extern int urs_list_node(unsigned long id, void *buf, unsigned long count, unsigned long *actual);
extern int urs_seek_list(unsigned long id, u64 offset, enum urs_seek_from from, unsigned long *newpos);

extern int urs_create_node(unsigned long id, char *name, enum urs_create_type type, unsigned int flags, char *target);
extern int urs_remove_node(unsigned long id);
extern int urs_rename_node(unsigned long id, char *name);


#endif
