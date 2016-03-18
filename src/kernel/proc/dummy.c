/*
 * Dummy kernel thread
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


void kernel_dummy_thread(ulong param)
{
    do {
    } while (1);
}

void kernel_demo_thread(ulong param)
{
    do {
    } while (1);
}
