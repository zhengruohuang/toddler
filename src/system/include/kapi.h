#ifndef __SYSTEM_INCLUDE_KAPI__
#define __SYSTEM_INCLUDE_KAPI__

#include "common/include/data.h"
#include "common/include/syscall.h"


extern void kapi_init();

extern asmlinkage void kapi_write_handler(msg_t *msg);


#endif
