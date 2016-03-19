#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/apic.h"
#include "hal/include/cpu.h"


void asmlinkage wrap_kernel_map(ulong addr, size_t size)
{
    kernel_direct_map_array(addr, size, 0);
}

int asmlinkage wrap_user_map(ulong page_dir, ulong vaddr, ulong paddr, ulong size, int exec, int write, int cacheable)
{
    return 0;   // FIXME
}

void asmlinkage wrap_halt()
{
    halt();
}

void asmlinkage wrap_sleep()
{
    __asm__ __volatile__
    (
        "hlt;"
        :
        :
    );
}

int asmlinkage wrap_get_cur_cpu_id()
{
    return  get_cpu_id();
}
