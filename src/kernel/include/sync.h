#ifndef __KERNEL_INCLUDE_SYNC__
#define __KERNEL_INCLUDE_SYNC__


#include "common/include/data.h"
#include "common/include/atomic.h"


/*
 * Generic atomic types
 */
typedef struct {
    volatile ulong value;
} atomic_t;


/*
 * Spin lock
 */
typedef struct {
    union {
        volatile ulong value;
        struct {
            volatile ulong locked       : 1;
            volatile ulong int_enabled  : 1;
        };
    };
} spinlock_t;


extern void spin_init(spinlock_t *lock);
extern void spin_lock(spinlock_t *lock);
extern void spin_unlock(spinlock_t *lock);
extern void spin_lock_int(spinlock_t *lock);
extern void spin_unlock_int(spinlock_t *lock);


/*
 * Spin-based readers-writer lock
 */
typedef struct {
    union {
        volatile ulong value;
        struct {
            volatile ulong locked       : 1;
            volatile ulong int_enabled  : 1;
            volatile ulong counter      : sizeof(ulong) * 8 - 2;
        };
    };
} rwlock_t;


#endif
