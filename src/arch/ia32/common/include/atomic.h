#ifndef __ARCH_IA32_COMMON_INCLUDE_ATOMIC__
#define __ARCH_IA32_COMMON_INCLUDE_ATOMIC__


#include "common/include/data.h"


/*
 * Atomic types
 */
#define SPIN_LOCK   0
#define RW_LOCK     1

typedef struct {
    volatile ulong value;
} atomic_t;

typedef struct {
    volatile ulong value;
} spinlock_t;

typedef struct {
    union {
        volatile ulong value;
        struct {
            volatile ulong locked   : 1;
            volatile ulong counter  : sizeof(ulong) * 8 - 1;
        };
    };
} rwlock_t;


/*
 * Comapre and swap
 */
static inline int atomic_cas(void *target, ulong old_value, ulong new_value)
{
    ulong result;
    
    __asm__ __volatile__
    (
        "lock cmpxchg    %%ebx, (%%esi)"
        : "=a" (result)
        : "a" (old_value), "b" (new_value), "S" (target)
    );
    
    return (result == old_value ? 1 : 0);
}


/*
 * Spin lock
 */
static inline void spin_lock(spinlock_t *lock)
{
    do {
        do {
        } while (lock->value);
    } while (!atomic_cas(&lock->value, 0, 1));
}

static inline void spin_unlock(spinlock_t *lock)
{
    lock->value = 0;
}

/*
 * Reader-wtire lock
 */
static inline void read_lock(rwlock_t *lock)
{
    rwlock_t old_val, new_val;
    
    do {
        do {
        } while (lock->locked);
        
        old_val.value = lock->value;
        new_val.locked = 0;
        new_val.counter = old_val.counter + 1;
    } while(!atomic_cas(&lock->value, old_val.value, new_val.value));
}

static inline void write_lock(rwlock_t *lock)
{
    rwlock_t new_val;
    
    do {
        do {
        } while (lock->value);
        
        new_val.locked = 1;
        new_val.counter = 0;
    } while(!atomic_cas(&lock->value, 0, new_val.value));
}

static inline void read_unlock(rwlock_t *lock)
{
    rwlock_t old_val, new_val;
    
    do {
        do {
        } while (lock->locked);
        
        old_val.value = lock->value;
        new_val.locked = 0;
        new_val.counter = old_val.counter - 1;
    } while(!atomic_cas(&lock->value, old_val.value, new_val.value));
}

static inline void write_unlock(rwlock_t *lock)
{
    lock->value = 0;
}


/*
 * Memory barriers
 */
static inline void membar()
{
    __asm__ __volatile__
    (
        "mfence"
        :
        :
    );
}

static inline void readbar()
{
    __asm__ __volatile__
    (
        "lfence"
        :
        :
    );
}

static inline void writebar()
{
    __asm__ __volatile__
    (
        "sfence"
        :
        :
    );
}


#endif
