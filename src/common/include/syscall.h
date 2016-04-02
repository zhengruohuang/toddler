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
#define IPC_DEST_KERNEL     0x0
#define IPC_FUNC_KAPI       0x0

struct msg {
    unsigned long dest_mailbox_id;
    unsigned long func_num;
    unsigned long func_param;
    int need_reply;
    unsigned long content_offset;
};

typedef volatile struct msg msg_t;


/*
 * KAPI
 */
#define KAPI_NONE           0x0

// write
#define KAPI_WRITE          0x0

struct msg_kapi_write_req {
    int fd;
    void *buf;
    size_t count;
};

struct msg_kapi_write_reply {
    int count;
};


#endif
