#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdarg.h"


#define BUF_SIZE    128


int asmlinkage ksnprintf(char *buf, size_t size, char *fmt, ...)
{
    // Args
    int ret = 0;
    va_list ap;
    va_start(ap, fmt);
    
    // Print
    ret = vsnprintf(buf, size, fmt, ap);
    
    va_end(ap);
    return ret;
}


int asmlinkage kprintf(char *fmt, ...)
{
    // Buf
    char buf[BUF_SIZE];
    size_t size = BUF_SIZE;
    
    // Args
    int ret = 0;
    va_list ap;
    va_start(ap, fmt);
    
    // Print
    ret = vsnprintf(buf, size, fmt, ap);
    syscall_kputs(buf);
    
    va_end(ap);
    return ret;
}
