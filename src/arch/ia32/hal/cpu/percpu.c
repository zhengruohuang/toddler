#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"


static int cur_per_cpu_offset = 0;


static void init_per_cpu_var(int *offset, size_t size)
{
    assert(cur_per_cpu_offset + size < PAGE_SIZE);
    
    *offset = cur_per_cpu_offset;
    cur_per_cpu_offset += size;
}

void *access_per_cpu_var(int *offset, size_t size)
{
    if (*offset == -1) {
        init_per_cpu_var(offset, size);
    }
    
    return (void *)(get_my_cpu_area_start_vaddr() + PER_CPU_KERNEL_DATA_START_OFFSET + *offset);
}
