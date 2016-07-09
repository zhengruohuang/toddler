#include "common/include/data.h"
#include "common/include/syscall.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/apic.h"
#include "hal/include/cpu.h"
#include "hal/include/mem.h"
#include "hal/include/exec.h"
#include "hal/include/int.h"
#include "hal/include/kernel.h"


int wrap_user_map(ulong page_dir_pfn, ulong vaddr, ulong paddr, ulong size, int exec, int write, int cacheable, int override)
{
    return user_indirect_map_array(page_dir_pfn, vaddr, paddr, size, exec, write, cacheable, override);
}

ulong wrap_get_paddr(ulong page_dir_pfn, ulong vaddr)
{
    return get_paddr(page_dir_pfn, vaddr);
}

int wrap_load_exe(ulong image_start, ulong dest_page_dir_pfn,
                             ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out)
{
    return load_exe(image_start, dest_page_dir_pfn, entry_out, vaddr_start_out, vaddr_end_out);
}

void wrap_init_addr_space(ulong page_dir_pfn)
{
    return init_user_page_dir(page_dir_pfn);
}

int wrap_get_cur_cpu_id()
{
    return get_cpu_id();
}

ulong wrap_io_in(ulong port, ulong size)
{
    return 0;
}

void wrap_io_out(ulong port, ulong size, ulong data)
{
    
}

void wrap_halt()
{
    halt();
}

void wrap_sleep()
{
    __asm__ __volatile__
    (
        "hlt;"
        :
        :
    );
}

#define GEN_INT_INSTR(vec) "int $" #vec ";"

void wrap_yield()
{
    unsigned long num = SYSCALL_YIELD;
    
    int succeed = 0;
    unsigned long param1 = 0, param2 = 0;
    unsigned long value1 = 0, value2 = 0;
    
    __asm__ __volatile__
    (
        GEN_INT_INSTR(0x7f)
        : "=a" (succeed), "=S" (value1), "=D" (value2)
        : "S"(num), "D" (param1), "a" (param2)
    );
}
