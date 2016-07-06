/*
 * Spin locks
 */


#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/sync.h"


/*
 * Spin lock
 */
void spin_init(spinlock_t *lock)
{
    lock->value = 0;
}

void spin_lock(spinlock_t *lock)
{
    do {
        do {
        } while (lock->value);
    } while (!atomic_cas(&lock->value, 0, 1));
    
    //kprintf("Locked: %p\n", lock);
}

void spin_unlock(spinlock_t *lock)
{
    assert(lock->locked);
    lock->value = 0;
    
    //kprintf("Unlocked: %p\n", lock);
}

void spin_lock_int(spinlock_t *lock)
{
    int enabled = hal->disable_local_interrupt();
    
    do {
        do {
        } while (lock->value);
    } while (!atomic_cas(&lock->value, 0, 1));
    
    spinlock_t newlock;
    newlock.locked = 1;
    newlock.int_enabled = enabled;
    
    assert(atomic_cas(&lock->value, 1, newlock.value));
}

void spin_unlock_int(spinlock_t *lock)
{
    int enabled = lock->int_enabled;
    
    assert(lock->locked);
    lock->value = 0;
    
//     if (enabled) {
//         kprintf("spin store: %d\n", enabled);
//     }
    
    hal->restore_local_interrupt(enabled);
}


/*
 * Readers-writer lock
 */
void read_lock(rwlock_t *lock)
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

void write_lock(rwlock_t *lock)
{
    rwlock_t new_val;
    
    do {
        do {
        } while (lock->value);
        
        new_val.locked = 1;
        new_val.counter = 0;
    } while(!atomic_cas(&lock->value, 0, new_val.value));
}

void read_unlock(rwlock_t *lock)
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

void write_unlock(rwlock_t *lock)
{
    lock->value = 0;
}
