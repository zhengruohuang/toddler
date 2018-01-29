#include "common/include/data.h"
#include "common/include/elf32.h"
#include "kernel/include/hal.h"
#include "kernel/include/lib.h"
#include "kernel/include/mem.h"
#include "kernel/include/exec.h"


/*
 * Load ELF executable
 */
int load_elf_exec(
    ulong image_start, ulong dest_page_dir_pfn,
    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out)
{
    ulong i, j;
    ulong vaddr_start = 0;
    ulong vaddr_end = 0;
    ulong entry = 0;
    
    elf_native_header_t *elf_header = (elf_native_header_t *)image_start;
    elf_native_program_t *prog_header;
//     struct elf32_header *elf_header = (struct elf32_header *)image_start;
//     struct elf32_program *prog_header;
    
    kprintf("\tLoad ELF image @ %p, page dir PFN %p\n",
            (void *)image_start, (void *)dest_page_dir_pfn);
    
    // For every segment, map and load
    for (i = 0; i < elf_header->elf_phnum; i++) {
        kprintf("\t\tSegment #%d\n", (int)i);
        
        // Get program header
        if (i) {
            prog_header = (void *)((ulong)prog_header + (ulong)elf_header->elf_phentsize);
        } else {
            prog_header = (void *)(image_start + elf_header->elf_phoff);
        }
        
        // Map the segment to destination's vaddr space
        if (prog_header->program_memsz) {
            kprintf("\t\t\tMapping ... ");
            
            ulong s = (ulong)prog_header->program_vaddr;
            s = ALIGN_DOWN(s, PAGE_SIZE);
            
            ulong e = (ulong)prog_header->program_vaddr + prog_header->program_memsz;
            e = ALIGN_UP(e, PAGE_SIZE);
            
            for (j = s; j < e; j += PAGE_SIZE) {
                if (hal->get_paddr(dest_page_dir_pfn, j)) {
                    continue;
                }
                
                ulong ppfn = palloc(1);
                ulong paddr = PFN_TO_ADDR(ppfn);
                
                kprintf(" (virt @ %p, phys @ %p) ", (void *)j, (void *)paddr);
                int mapped = hal->map_user(
                    dest_page_dir_pfn, j, paddr, PAGE_SIZE,
                    1, 1, 1, 0
                );
                assert(mapped);
                
                // Zero the memory
                memzero((void *)paddr, PAGE_SIZE);
            }
            
            kprintf("%d bytes\n", prog_header->program_memsz);
        }
        
        // Copy the program data
        if (prog_header->program_filesz) {
            assert(prog_header->program_memsz >= prog_header->program_filesz);
            
            kprintf("\t\t\tCopy ... (virt @ %p, phys @ %p) ",
                    (void *)(ulong)prog_header->program_vaddr,
                    (void *)((ulong)image_start + (ulong)prog_header->program_offset)
            );
            
            copy_to_user(
                (ulong)image_start + (ulong)prog_header->program_offset,
                (ulong)prog_header->program_filesz,
                dest_page_dir_pfn, (ulong)prog_header->program_vaddr
            );
            
            kprintf("%d bytes\n", prog_header->program_filesz);
        }
        
        // Get the start and end of vaddr
        if (prog_header->program_vaddr + prog_header->program_memsz > vaddr_end) {
            vaddr_end = (ulong)prog_header->program_vaddr + (ulong)prog_header->program_memsz;
        }
        
        if (prog_header->program_vaddr < vaddr_start) {
            vaddr_start = prog_header->program_vaddr;
        }
    }
    
    // Get entry point
    entry = elf_header->elf_entry;
    
    kprintf("\t\tEntry @ %p\n", (void *)entry);
    kprintf("\t\tVaddr start @ %p, end @ %p\n", (void *)vaddr_start, (void *)vaddr_end);
    
    // Done
    *entry_out = entry;
    *vaddr_start_out = vaddr_start;
    *vaddr_end_out = vaddr_end;
    
    return 1;
}
