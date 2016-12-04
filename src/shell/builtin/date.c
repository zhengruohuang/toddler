#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/time.h"
#include "shell/include/shell.h"


int date(int argc, char **argv)
{
    int i;
    
    time_t t = time();
    kprintf("Timestamp: %lu%lu\n",
        (unsigned long)(t >> (sizeof(unsigned long) * 8)),
        (unsigned long)t
    );
    
    return EOK;
}
