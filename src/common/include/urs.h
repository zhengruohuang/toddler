#ifndef __COMMON_INCLUDE_URS__
#define __COMMON_INCLUDE_URS__


enum urs_seek_from {
    seek_from_begin,
    seek_from_cur_fwd,
    seek_from_cur_bwd,
    seek_from_end,
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


#endif
