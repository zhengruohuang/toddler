#ifndef __ARCH_IA32_HAL_INCLUDE_EXEC__
#define __ARCH_IA32_HAL_INCLUDE_EXEC__


#include "common/include/data.h"


/*
 * Copy
 */
extern void copy_to_user(
    ulong src_start, ulong len,
    ulong user_page_dir_pfn, ulong user_start
);


/*
 * Load
 */
extern int load_elf_exe(
    ulong image_start, ulong dest_page_dir_pfn,
    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out
);

extern int load_exe(
    ulong image_start, ulong dest_page_dir_pfn,
    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out
);


#endif
