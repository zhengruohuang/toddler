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
#define SYSCALL_YIELD           0x3

// I/O ports
// On systems with only memory-mapped I/O, making these calls is unnecessary
#define SYSCALL_IO_OUT          0x10
#define SYSCALL_IO_IN           0x11

// Thread control block
// On systems with fast TCB support, making these calls is unecessary
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

// Message param type
#define MSG_PARAM_EMPTY         0x0
#define MSG_PARAM_VALUE         0x1
#define MSG_PARAM_BUFFER        0x2

struct msg_param {
    int type;
    
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
#define KAPI_NONE               0x0

#define KAPI_PROCESS_CREATE     0x10
#define KAPI_PROCESS_EXIT       0x11
#define KAPI_PROCESS_KILL       0x12
#define KAPI_PROCESS_ID         0x13

#define KAPI_THREAD_CREATE      0x20
#define KAPI_THREAD_EXIT        0x21
#define KAPI_THREAD_KILL        0x22
#define KAPI_THREAD_ID          0x23

#define KAPI_FILE_OPEN          0x30
#define KAPI_FILE_CLOSE         0x31
#define KAPI_FILE_WRITE         0x32
#define KAPI_FILE_READ          0x33

#define KAPI_TIME_TIMES         0x40
#define KAPI_TIME_DAY           0x41

#define KAPI_INTERRUPT_REG      0x50
#define KAPI_INTERRUPT_UNREG    0x51


#endif
