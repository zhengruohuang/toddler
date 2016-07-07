

enum urs_dispatch_type {
    udisp_none,
    udisp_func,
    udisp_msg,
    udisp_list,
};

enum urs_node_type {
    unode_none,
    unode_obj,
    unode_list,
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
        
        struct urs_node *list_head;
    };
};

struct urs_node {
    char *name;
    enum urs_node_type type;
    struct urs_dispatch dispatch;
};

struct urs_namespace {
    char *name;
    enum urs_node_type type;
    struct urs_dispatch dispatch;
};
