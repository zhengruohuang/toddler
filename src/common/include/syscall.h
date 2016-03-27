#ifndef __COMMON_INCLUDE_SYSCALL__
#define __COMMON_INCLUDE_SYSCALL__


#include "common/include/data.h"


#define SYSCALL_PING        0
#define SYSCALL_KPUTS       1

#define SYSCALL_CREATE_MSG  2
#define SYSCALL_SEND        3
#define SYSCALL_SENDRECV    4
#define SYSCALL_RECV        5


/*
 * A message
 */
struct ipc_msg {
    ulong sender_id;
    
};


#endif
