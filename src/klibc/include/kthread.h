#ifndef __KLIBC_INCLUDE_KTHREAD__
#define __KLIBC_INCLUDE_KTHREAD__


#include "common/include/data.h"
#include "klibc/include/stdarg.h"


/*
 * TLS
 */
extern unsigned long ktls_alloc(size_t size);
extern void *ktls_access(unsigned long tls_offset);
extern void init_tls();


/*
 * Thread control
 */
struct kthread {
    volatile unsigned long thread_id;
    volatile unsigned long return_value;
    
    volatile int started;
    volatile int terminated;
};

typedef volatile struct kthread kthread_t;

typedef unsigned long (*start_routine_t)(unsigned long);

extern int kthread_create(kthread_t *thread, start_routine_t start, unsigned long arg);
extern void kthread_exit(unsigned long retval);
void kthread_kill(kthread_t *thread, unsigned long retval);
void init_kthread();


/*
 * Mutex
 */
#define KTHREAD_MUTEX_INIT 0

typedef volatile unsigned long kthread_mutex_t;

extern void kthread_mutex_init(kthread_mutex_t *mutex);
extern void kthread_mutex_destroy(kthread_mutex_t *mutex);

extern void kthread_mutex_lock(kthread_mutex_t *mutex);
extern int kthread_mutex_trylock(kthread_mutex_t *mutex);
extern int kthread_mutex_unlock(kthread_mutex_t *mutex);


#endif
