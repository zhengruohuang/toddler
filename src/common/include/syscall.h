#ifndef __COMMON_INCLUDE_SYSCALL__
#define __COMMON_INCLUDE_SYSCALL__


#include "common/include/data.h"


/*
 * System call
 */
// For system and tesing
#define SYSCALL_NONE        0x0
#define SYSCALL_PING        0x1
#define SYSCALL_KPUTS       0x2

// Thread control block
// On systems with fast TCB support, calling this syscall is unecessary
#define SYSCALL_GET_TCB     0x10

// IPC
#define SYSCALL_SEND        0x20
#define SYSCALL_SENDRECV    0x21
#define SYSCALL_RECV        0x22

// IO ports
// On systems with only memory-mapped IO, calling these syscalls is unnecessary
#define SYSCALL_IO_OUT      0x30
#define SYSCALL_IO_IN       0x31


/*
 * IPC
 */
// Default destinations
#define IPC_DEST_NONE       0x0
#define IPC_DEST_KERNEL     0x1

// Default function types
#define IPC_FUNC_NONE       0x0
#define IPC_FUNC_KAPI       0x1

enum msg_param_type {
    msg_param_value,
    msg_param_addr,
};

struct msg_param {
    enum msg_param_type type;
    
    union {
        unsigned long value;
        
        struct {
            void *vaddr;
            unsigned long size;
        };
    };
};

struct msg {
    unsigned long dest_mailbox_id;
    unsigned long func_type;
    unsigned long func_num;
    int need_reply;
    int param_count;
    struct msg_param params[10];
};

typedef volatile struct msg msg_t;


/*
 * KAPI
 */
#define KAPI_NONE           0x0
#define KAPI_WRITE          0x1


#endif
