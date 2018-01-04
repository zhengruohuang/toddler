#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/debug.h"
#include "hal/include/bit.h"


static int cur_per_cpu_offset = 0;


/*
 * Arch-specific get_my_cpu_area_start_vaddr
 */
weak_func ulong get_my_cpu_area_start_vaddr()
{
    return 0;
}


static void init_per_cpu_var(int *offset, size_t size)
{
    assert(cur_per_cpu_offset + size < PAGE_SIZE);
    
    *offset = cur_per_cpu_offset;
    cur_per_cpu_offset += ALIGN_UP(size, sizeof(ulong));
}

void *access_per_cpu_var(int *offset, size_t size)
{
    if (*offset == -1) {
        init_per_cpu_var(offset, size);
    }
    
    return (void *)(get_my_cpu_area_start_vaddr() + PER_CPU_DATA_START_OFFSET + *offset);
}
