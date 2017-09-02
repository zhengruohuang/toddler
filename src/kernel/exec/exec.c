#include "common/include/data.h"
#include "kernel/include/exec.h"


int load_exec(
    ulong image_start, ulong dest_page_dir_pfn,
    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out
)
{
    return load_elf_exec(image_start, dest_page_dir_pfn, entry_out, vaddr_start_out, vaddr_end_out);
}

