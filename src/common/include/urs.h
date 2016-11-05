#ifndef __COMMON_INCLUDE_URS__
#define __COMMON_INCLUDE_URS__


/*
 * Seek data and list
 */
enum urs_seek_from {
    seek_from_begin,
    seek_from_cur_fwd,
    seek_from_cur_bwd,
    seek_from_end,
};


/*
 * URS operations
 */
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


/*
 * Registration
 */
enum urs_reg_type {
    ureg_none,
    ureg_func,
    ureg_msg,
};

struct urs_reg_entry {
    enum urs_reg_type type;
    
    union {
        void *func;
        struct {
            unsigned long msg_opcode;
            unsigned long msg_func_num;
        };
    };
};

struct urs_reg_ops {
    unsigned long mbox_id;
    struct urs_reg_entry entries[uop_count];
};


/*
 * Creation
 */
enum urs_create_type {
    ucreate_none,
    ucreate_node,
    ucreate_dyn_link,
    ucreate_sym_link,
    ucreate_hard_link,
};


#endif
