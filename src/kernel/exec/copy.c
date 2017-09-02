#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/lib.h"


void cross_as_copy(
    ulong src_page_dir_pfn, ulong src_start, ulong len,
    ulong dest_page_dir_pfn, ulong dest_start)
{
    ulong i;
    ulong src_end = src_start + len;
    
    // Round down src start and dest start
    src_start = ALIGN_DOWN(src_start, sizeof(ulong));
    dest_start = ALIGN_DOWN(dest_start, sizeof(ulong));
    src_end = ALIGN_UP(src_end, sizeof(ulong));
    
    // Calculate actual length
    len = src_end - src_start;
    
    // Do the actual copy
    ulong cur_src_paddr, cur_dest_paddr;
    for (i = 0; i < len; i += sizeof(ulong)) {
        // Get physical address
        cur_src_paddr = hal->get_paddr(src_page_dir_pfn, src_start + i);
        cur_dest_paddr = hal->get_paddr(dest_page_dir_pfn, dest_start + i);
        assert(cur_src_paddr && cur_dest_paddr);
        
//         kprintf("vsrc: %p, psrc: %p, vdest: %p, pdest: %p",
//                 (void *)(src_start + i), (void *)cur_src_paddr,
//                 (void *)(dest_start + i), (void *)cur_dest_paddr);
        
        // Copy the value
        ulong word = *((ulong *)cur_src_paddr);
        *((ulong *)cur_dest_paddr) = word;
        
//         kprintf("word: %p\n", (void *)word);
    }
}

void copy_to_user(
    ulong src_start, ulong len,
    ulong user_page_dir_pfn, ulong user_start)
{
    ulong i;
    ulong src_end = src_start + len;
    
    // Round down src start and dest start
    src_start = ALIGN_DOWN(src_start, sizeof(ulong));
    user_start = ALIGN_DOWN(user_start, sizeof(ulong));
    src_end = ALIGN_UP(src_end, sizeof(ulong));
    
    // Calculate actual length
    len = src_end - src_start;
    
    // Do the actual copy
    ulong cur_src_paddr, cur_dest_paddr;
    for (i = 0; i < len; i += sizeof(ulong)) {
        // Get physical address
        cur_src_paddr = src_start + i;
        cur_dest_paddr = hal->get_paddr(user_page_dir_pfn, user_start + i);
        assert(cur_src_paddr && cur_dest_paddr);
        
//         kprintf("psrc: %p, vdest: %p, pdest: %p",
//                 (void *)cur_src_paddr,
//                 (void *)(user_start + i), (void *)cur_dest_paddr);
        
        // Copy the value
        ulong word = *((ulong *)cur_src_paddr);
        *((ulong *)cur_dest_paddr) = word;
        
//         kprintf("word: %p\n", (void *)word);
    }
}
