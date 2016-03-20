#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/elf32.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"
#include "hal/include/kernel.h"
#include "hal/include/exec.h"


/*
 * Load ELF executable
 */
int load_elf_exe(
    ulong image_start, ulong dest_page_dir_pfn,
    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out)
{
    ulong i, j;
    ulong vaddr_start = 0;
    ulong vaddr_end = 0;
    ulong entry = 0;
    
    struct elf32_elf_header *elf_header = (struct elf32_elf_header *)image_start;
    struct elf32_program_header *prog_header;
    
    kprintf("\tLoad ELF image at %p, page dir PFN %d\n", image_start, dest_page_dir_pfn);
    
    // For every segment, map and load
    for (i = 0; i < elf_header->elf_phnum; i++) {
        kprintf("\t\tSegment #%d\n", i);
        
        // Get program header
        prog_header = (struct elf32_program_header *)(image_start + elf_header->elf_phoff + elf_header->elf_phentsize * i);
        
        // Map the segment to destination's vaddr space
        if (prog_header->program_memsz) {
            kprintf("\t\t\tMapping ... ");
            
            ulong s = (ulong)prog_header->program_vaddr;
            s /= PAGE_SIZE;
            s *= PAGE_SIZE;
            
            ulong e = (ulong)prog_header->program_vaddr + prog_header->program_memsz;
            if (e % PAGE_SIZE) {
                e /= PAGE_SIZE;
                e++;
                e *= PAGE_SIZE;
            }
            
            for (j = s; j < e; j += PAGE_SIZE) {
                if (get_paddr(dest_page_dir_pfn, j)) {
                    continue;
                }
                
                ulong ppfn = kernel->palloc(1);
                ulong paddr = PFN_TO_ADDR(ppfn);
                
                int mapped = user_indirect_map_array(
                    dest_page_dir_pfn, j, paddr, PAGE_SIZE,
                    1, 1, 1, 0
                );
                
                assert(mapped);
            }
            
            kprintf("%d bytes\n", prog_header->program_memsz);
        }
        
        // Copy the program data
        if (prog_header->program_filesz) {
            assert(prog_header->program_memsz >= prog_header->program_filesz);
            
            kprintf("\t\t\tCopy ... ");
            
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
    
    entry = elf_header->elf_entry;
    
    kprintf("\t\t\tEntry at %p\n", entry);
    kprintf("\t\t\tVaddr start: %p, end: %p\n", vaddr_start, vaddr_end);
    
    *entry_out = entry;
    *vaddr_start_out = vaddr_start;
    *vaddr_end_out = vaddr_end;
    
    return 1;
}
