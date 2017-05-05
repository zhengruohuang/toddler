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
#define SYSCALL_TIME            0x3
#define SYSCALL_YIELD           0x4

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
#define IPC_OPCODE_ACTION       0x2

// Message param type
#define MSG_PARAM_EMPTY         0x0
#define MSG_PARAM_VALUE         0x1
#define MSG_PARAM_VALUE64       0x2
#define MSG_PARAM_BUFFER        0x3

struct msg_param {
    int type;
    
    union {
        unsigned long value;
        u64 value64;
        struct {
            int offset;
            int size;
        };
    };
};

struct msg {
    // Filled by kernel
    unsigned long sender_proc_id;
    unsigned long sender_thread_id;
    
    // Filled by user
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

// Process
#define KAPI_PROCESS_CREATE     0x10
#define KAPI_PROCESS_STARTED    0x11
#define KAPI_PROCESS_EXIT       0x12
#define KAPI_PROCESS_KILL       0x13
#define KAPI_PROCESS_ID         0x14
#define KAPI_PROCESS_MONITOR    0x15

// Thread
#define KAPI_THREAD_CREATE      0x20
#define KAPI_THREAD_EXIT        0x21
#define KAPI_THREAD_KILL        0x22
#define KAPI_THREAD_ID          0x23

// Time
#define KAPI_TIME_TIMES         0x30
#define KAPI_TIME_DAY           0x31

// Interrupt
#define KAPI_INTERRUPT_REG      0x40
#define KAPI_INTERRUPT_UNREG    0x41

// Heap
#define KAPI_HEAP_END_GET       0x50
#define KAPI_HEAP_END_SET       0x51
#define KAPI_HEAP_END_GROW      0x52
#define KAPI_HEAP_END_SHRINK    0x53

// URS
#define KAPI_URS_OPEN           0x60
#define KAPI_URS_CLOSE          0x61
#define KAPI_URS_READ           0x62
#define KAPI_URS_WRITE          0x63
#define KAPI_URS_TRUNCATE       0x64
#define KAPI_URS_SEEK_DATA      0x65
#define KAPI_URS_LIST           0x66
#define KAPI_URS_SEEK_LIST      0x67
#define KAPI_URS_CREATE         0x68
#define KAPI_URS_REMOVE         0x69
#define KAPI_URS_RENAME         0x6a
#define KAPI_URS_STAT           0x6b
#define KAPI_URS_IOCTL          0x6c

#define KAPI_URS_REG_SUPER      0x70
#define KAPI_URS_REG_OP         0x71

// KMap
#define KAPI_KMAP               0x80

// Temporary stdio KAPIs, they eventually should be implemented through URS
#define KAPI_STDIN_READ         0x1000
#define KAPI_STDOUT_WRITE       0x1001
#define KAPI_STDERR_WRITE       0x1002

// User messages
#define USER_MSG_NUM_BASE       0x8000000

#endif
