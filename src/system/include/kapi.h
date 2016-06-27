#ifndef __KAPI_H__
#define __KAPI_H__

#include "common/include/data.h"
#include "common/include/syscall.h"


extern int kapi_init();
extern int kapi_write(int fd, void *buf, size_t count);


#endif
