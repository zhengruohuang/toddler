#ifndef __COMMON_INCLUDE_SYSCALL__
#define __COMMON_INCLUDE_SYSCALL__


#include "common/include/data.h"


/*
 * System call
 */
// For system and tesing
#define SYSCALL_NONE            0x0
#define SYSCALL_PING            0x1
#define SYSCALL_KPUTS           0x2

// IO ports
// On systems with only memory-mapped IO, calling these syscalls is unnecessary
#define SYSCALL_IO_OUT          0x10
#define SYSCALL_IO_IN           0x11

// Thread control block
// On systems with fast TCB support, calling this syscall is unecessary
#define SYSCALL_GET_TCB         0x20

// IPC
#define SYSCALL_REG_MSG_HANDLER     0x30
#define SYSCALL_UNREG_MSG_HANDLER   0x31
#define SYSCALL_SEND            0x32
#define SYSCALL_REQUEST         0x33
#define SYSCALL_RECV            0x34
#define SYSCALL_REPLY           0x35
#define SYSCALL_RESPOND         0x36

// KAPI
#define SYSCALL_REG_KAPI_SERVER     0x40
#define SYSCALL_UNREG_KAPI_SERVER   0x41


/*
 * IPC
 */
// Default mailbox
#define IPC_MAILBOX_NONE            0x0
#define IPC_MAILBOX_KERNEL          0x1
#define IPC_MAILBOX_THIS_PROCESS    0x2

// Default opcode
#define IPC_OPCODE_NONE         0x0
#define IPC_OPCODE_KAPI         0x1

enum msg_param_type {
    msg_param_empty = 0,
    msg_param_value = 1,
    msg_param_buffer = 2,
};

struct msg_param {
    enum msg_param_type type;
    
    union {
        unsigned long value;
        
        struct {
            int offset;
            int size;
        };
    };
};

struct msg {
    unsigned long mailbox_id;
    unsigned long opcode;
    unsigned long func_num;
    int msg_size;
    int param_count;
    struct msg_param params[8];
} packedstruct;

typedef volatile struct msg msg_t;
typedef asmlinkage void (*msg_handler_t)(msg_t *msg);


/*
 * KAPI
 */
#define KAPI_NONE           0x0
#define KAPI_WRITE          0x1


#endif
