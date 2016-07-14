#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/proc.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"


static kthread_mutex_t tls_mutex = KTHREAD_MUTEX_INIT;
static unsigned long tls_size = PAGE_SIZE;
static unsigned long cur_tls_offset = 8;


unsigned long ktls_alloc(size_t size)
{
    unsigned long result = 0;
    
    kthread_mutex_lock(&tls_mutex);
    
    if (cur_tls_offset + size <= tls_size) {
        result = cur_tls_offset;
        cur_tls_offset += size;
    }
    
    kthread_mutex_unlock(&tls_mutex);
    
    return result;
}

void *ktls_access(unsigned long tls_offset)
{
    struct thread_control_block *tcb = get_tcb();
    return tcb->tls + tls_offset;
}

void init_tls()
{
}
