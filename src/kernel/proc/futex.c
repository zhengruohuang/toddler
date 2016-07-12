/*
 * Thread fast user-space mutex
 */

#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/ds.h"
#include "kernel/include/proc.h"


static int futex_table_entry_salloc_id = -1;
static int thread_list_node_salloc_id = -1;


/*
 * Init
 */
void init_futex()
{
}


/*
 * Simple futex
 */
void sfutex_wait(ulong vaddr, struct thread *t)
{
}

void sfutex_release(ulong vaddr, struct thread *t)
{
}
