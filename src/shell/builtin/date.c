#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/time.h"
#include "shell/include/shell.h"


int date(int argc, char **argv)
{
    time_t t = time();
    
#if (ARCH_WIDTH == 64)
    kprintf("Timestamp: %lu\n",(unsigned long)t);
#else
    kprintf("Timestamp: %lu%lu\n",
        (unsigned long)(t >> (sizeof(unsigned long) * 8)),
        (unsigned long)t
    );
#endif
    
    return EOK;
}
