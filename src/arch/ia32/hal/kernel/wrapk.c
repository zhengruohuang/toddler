#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/apic.h"
#include "hal/include/cpu.h"
#include "hal/include/mem.h"
#include "hal/include/exec.h"


int asmlinkage wrap_user_map(ulong page_dir_pfn, ulong vaddr, ulong paddr, ulong size, int exec, int write, int cacheable, int override)
{
    return user_indirect_map_array(page_dir_pfn, vaddr, paddr, size, exec, write, cacheable, override);
}

ulong asmlinkage wrap_get_paddr(ulong page_dir_pfn, ulong vaddr)
{
    return get_paddr(page_dir_pfn, vaddr);
}

int asmlinkage wrap_load_exe(ulong image_start, ulong dest_page_dir_pfn,
                             ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out)
{
    return load_exe(image_start, dest_page_dir_pfn, entry_out, vaddr_start_out, vaddr_end_out);
}

void asmlinkage wrap_init_addr_space(ulong page_dir_pfn)
{
    return init_user_page_dir(page_dir_pfn);
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

ulong asmlinkage wrap_io_in(ulong port, ulong size)
{
    return 0;
}

void asmlinkage wrap_io_out(ulong port, ulong size, ulong data)
{
    
}
