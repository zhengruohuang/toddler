#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/elf32.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"
#include "hal/include/exec.h"


static void cross_as_copy(
    ulong src_page_dir_pfn, ulong src_start, ulong length,
    ulong dest_page_dir_pfn, ulong dest_start)
{
    ulong i;
    
    ulong src_end = src_start + length;
    
    // Round down src start and dest start
    src_start /= sizeof(ulong);
    src_start *= sizeof(ulong);
    
    dest_start /= sizeof(ulong);
    dest_start *= sizeof(ulong);
    
    // Round up src end
    if (src_end % sizeof(ulong)) {
        src_end /= sizeof(ulong);
        src_end++;
        src_end *= sizeof(ulong);
    }
    
    // Calculate actual length
    length = src_end - src_start;
    
    // Do the actual copy
    ulong cur_src_paddr, cur_dest_paddr;
    for (i = 0; i < length; i += sizeof(ulong)) {
        // Get physical address
        cur_src_paddr = get_paddr(src_page_dir_pfn, src_start + i);
        cur_dest_paddr = get_paddr(dest_page_dir_pfn, dest_start + i);
        assert(cur_src_paddr && cur_dest_paddr);
        
        // Copy the value
        ulong word = *((ulong *)cur_src_paddr);
        *((ulong *)cur_dest_paddr) = word;
        
        kprintf("%h ", word);
    }
}

void copy_to_user(
    ulong src_start, ulong len,
    ulong user_page_dir_pfn, ulong user_start)
{
    cross_as_copy(KERNEL_PDE_PFN, src_start, len, user_page_dir_pfn, user_start);
}


int load_exe(
    ulong image_start, ulong dest_page_dir_pfn,
    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out
)
{
    return load_elf_exe(image_start, dest_page_dir_pfn, entry_out, vaddr_end_out, vaddr_end_out);
}
