#ifndef __KERNEL_INCLUDE_EXEC__
#define __KERNEL_INCLUDE_EXEC__


#include "common/include/data.h"


/*
 * ELF
 */
extern int load_elf_exec(
    ulong image_start, ulong dest_page_dir_pfn,
    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out);

/*
 * Copy
 */
extern void cross_as_copy(
    ulong src_page_dir_pfn, ulong src_start, ulong len,
    ulong dest_page_dir_pfn, ulong dest_start);
extern void copy_to_user(
    ulong src_start, ulong len,
    ulong user_page_dir_pfn, ulong user_start);


/*
 * Load executable
 */
extern int load_exec(
    ulong image_start, ulong dest_page_dir_pfn,
    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out
);


#endif

