#include "common/include/data.h"
#include "common/include/syscall.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/mem.h"
#include "hal/include/int.h"
#include "hal/include/kernel.h"


int wrap_user_map(ulong page_dir_pfn, ulong vaddr, ulong paddr, ulong size, int exec, int write, int cacheable, int override)
{
    return user_indirect_map_array(page_dir_pfn, vaddr, paddr, size, exec, write, cacheable, override);
}

int wrap_user_unmap(ulong page_dir_pfn, ulong vaddr, ulong paddr, ulong size)
{
    return user_indirect_unmap_array(page_dir_pfn, vaddr, paddr, size);
}

ulong wrap_get_paddr(ulong page_dir_pfn, ulong vaddr)
{
    return get_paddr(page_dir_pfn, vaddr);
}

int wrap_load_exe(ulong image_start, ulong dest_page_dir_pfn,
                             ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out)
{
    return 0;
//     return load_exe(image_start, dest_page_dir_pfn, entry_out, vaddr_start_out, vaddr_end_out);
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

void wrap_invalidate_tlb(ulong asid, ulong vaddr, size_t size)
{
    invalidate_tlb_array(asid, vaddr, size);
}

void wrap_halt()
{
    halt();
}

void wrap_sleep()
{
    while (1);
}

// #define GEN_INT_INSTR(vec) "int $" #vec ";"

void wrap_yield()
{
//     unsigned long num = SYSCALL_YIELD;
//     
//     int succeed = 0;
//     unsigned long param1 = 0, param2 = 0;
//     unsigned long value1 = 0, value2 = 0;
//     
//     __asm__ __volatile__
//     (
//         GEN_INT_INSTR(0x7f)
//         : "=a" (succeed), "=S" (value1), "=D" (value2)
//         : "S"(num), "D" (param1), "a" (param2)
//         : "memory"
//     );
}

int wrap_ksyscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2)
{
    int succeed = 0;
//     unsigned long value1 = 0, value2 = 0;
//     
//     __asm__ __volatile__
//     (
//         GEN_INT_INSTR(0x7f)
//         : "=a" (succeed), "=S" (value1), "=D" (value2)
//         : "S"(num), "D" (param1), "a" (param2)
//         : "memory"
//     );
//     
//     if (out1) {
//         *out1 = value1;
//     }
//     
//     if (out2) {
//         *out2 = value2;
//     }
    
    return succeed;
}
