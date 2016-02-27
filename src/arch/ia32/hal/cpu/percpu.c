#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"


static void init_per_cpu_var(int *id, size_t size)
{
}

void *access_per_cpu_var(int *id, size_t size)
{
    if (*id == -1) {
        init_per_cpu_var(id, size);
    }
    
    return NULL;
}
