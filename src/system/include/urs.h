#ifndef __SYSTEM_INCLUDE_URS__
#define __SYSTEM_INCLUDE_URS__


#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/stdstruct.h"


enum urs_seek_from {
    seek_from_begin,
    seek_from_cur_fwd,
    seek_from_cur_bwd,
    seek_from_end,
};

enum urs_link_type {
    ulink_none,
    ulink_absolute,
    ulink_relative,
};

enum urs_disp_type {
    udisp_none,
    udisp_func,
    udisp_msg,
};

enum urs_op_type {
    uop_none,
    
    uop_lookup,
    uop_open,
    uop_release,
    
    uop_read,
    uop_write,
    uop_truncate,
    uop_seek_data,
    
    uop_list,
    uop_seek_list,
    
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
    
    unsigned long long data_pos;
    unsigned long long data_size;
    
    unsigned long long list_pos;
    unsigned long long list_size;
    
    struct urs_node *node;
};


extern void init_urs();

extern unsigned long urs_register(char *path);
extern int urs_unregister(char *path);
extern int urs_register_op(
    unsigned long id, enum urs_op_type op, void *func,
    unsigned long mbox_id, unsigned long msg_opcode, unsigned long msg_func_num
);

extern unsigned long urs_open_node(char *path, unsigned int mode, unsigned long process_id);
extern int urs_close_node(unsigned long id, unsigned long process_id);

extern int urs_read_node(unsigned long id, void *buf, unsigned long count, unsigned long *actual);
extern int urs_write_node(unsigned long id, void *buf, unsigned long count, unsigned long *actual);
extern int urs_truncate_node(unsigned long id);
extern int urs_seek_data(unsigned long id, unsigned long offset, enum urs_seek_from from, unsigned long *newpos);

extern int urs_list_node(unsigned long id, void *buf, unsigned long count);
extern int urs_seek_list(unsigned long id, unsigned long long offset, enum urs_seek_from from, unsigned long *newpos);

extern int urs_create_node(unsigned long id, char *name);
extern int urs_remove_node(unsigned long id);
extern int urs_rename_node(unsigned long id, char *name);


#endif
