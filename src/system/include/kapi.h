#ifndef __SYSTEM_KAPI_H__
#define __SYSTEM_KAPI_H__

#include "common/include/data.h"
#include "common/include/syscall.h"


extern void kapi_init();

extern asmlinkage void kapi_write_handler(msg_t *msg);


#endif
