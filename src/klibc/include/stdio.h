#ifndef __KLIBC_INCLUDE_STDIO__
#define __KLIBC_INCLUDE_STDIO__


#include "common/include/data.h"
#include "klibc/include/stdarg.h"


extern int asmlinkage kprintf(char *fmt, ...);
extern int asmlinkage ksnprintf(char *buf, size_t size, char *fmt, ...);
extern int asmlinkage vsnprintf(char *buf, size_t size, char *fmt, va_list ap);


#endif
