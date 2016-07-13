#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"


int ktls_alloc(size_t size)
{
    return -1;
}

void *ktls_access(int tls_id)
{
    return NULL;
}

void init_tls()
{
}
