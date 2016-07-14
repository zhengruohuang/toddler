#include "common/include/data.h"
#include "common/include/atomic.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"


void kthread_mutex_init(kthread_mutex_t *mutex)
{
    *mutex = KTHREAD_MUTEX_INIT;
}

void kthread_mutex_destroy(kthread_mutex_t *mutex)
{
    *mutex = KTHREAD_MUTEX_INIT;
}


void kthread_mutex_lock(kthread_mutex_t *mutex)
{
    do {
        while (*mutex) {
            // kapi_futex_wait(mutex);
        }
    } while (!atomic_cas(mutex, 0, 0x1));
}

int kthread_mutex_trylock(kthread_mutex_t *mutex)
{
    if (*mutex) {
        return 0;
    }
    
    return atomic_cas(mutex, 0, 0x1);
}

int kthread_mutex_unlock(kthread_mutex_t *mutex)
{
    if (!(*mutex)) {
        return 0;
    }
    
    atomic_write(mutex, 0);
    // kapi_futex_release(mutex);
    return 1;
}
