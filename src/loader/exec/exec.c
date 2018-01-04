#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/coreimg.h"
#include "common/include/bootparam.h"
#include "common/include/elf32.h"
#include "loader/include/print.h"
#include "loader/include/lib.h"


enum image_load_type {
    LOAD_HAL,
    LOAD_KERNEL,
};

static void find_and_layout(struct coreimg_fat *coreimg, struct boot_parameters *bp,
                            char *name, int type, ulong offset_up, ulong offset_down)
{
    u32 i;
    
    // Find the file
    u32 file_count = coreimg->header.file_count;
    if (ARCH_BIG_ENDIAN != coreimg->header.big_endian) {
        file_count = swap_endian32(file_count);
    }
    
    struct coreimg_record *record = NULL;
    int found = 0;
    
    for (i = 0; i < file_count; i++) {
        record = &coreimg->records[i];
        if (!strcmp2(name, (char *)record->file_name, 20)) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        lprintf("Unable to find file: %s\n", name);
        panic();
    }
    
    lprintf("Loading image: %s\n", name);
    
    // Load the file
    u32 start_offset = record->start_offset;
    if (ARCH_BIG_ENDIAN != coreimg->header.big_endian) {
        start_offset = swap_endian32(start_offset);
    }
    
    elf_native_header_t *elf_header = (elf_native_header_t *)((ulong)start_offset + (ulong)coreimg);
    elf_native_program_t *header;
    ulong vaddr_end = 0;
    ulong target = 0;
    
    // For each segment
    for (i = 0; i < elf_header->elf_phnum; i++) {
        if (i) {
            header = (void *)((ulong)header + (ulong)elf_header->elf_phentsize);
        } else {
            header = (void *)((ulong)elf_header + (ulong)elf_header->elf_phoff);
        }
        
        // Rebase the target
        target = header->program_vaddr;
        if (header->program_memsz || header->program_filesz) {
            target += offset_up;
            target -= offset_down;
            lprintf("\tRebased @ %lx -> %lx\n", header->program_vaddr, target);
        }
        
        // Zero the memory
        if (header->program_memsz) {
            lprintf("\tZero memory @ %lx, size: %lx\n", target,
                    (ulong)header->program_memsz);
            memzero((void *)target, header->program_memsz);
        }
        
        // Copy the program data
        if (header->program_filesz) {
            lprintf("\tCopy section @ %lx -> %lx, size: %lx\n",
                    (ulong)header->program_offset + (ulong)elf_header,
                    target, (ulong)header->program_filesz
            );
            
            memcpy(
                (void *)target,
                (void *)(ulong)(header->program_offset + (ulong)elf_header),
                header->program_filesz
            );
        }
        
        // Get the end of virtual address
        if (header->program_vaddr + header->program_memsz > vaddr_end) {
            vaddr_end = header->program_vaddr + header->program_memsz;
        }
    }
    
    // We just loaded HAL
    if (type == LOAD_HAL) {
        // HAL Virtual Address End: Align to page size
        vaddr_end = ALIGN_UP(vaddr_end, PAGE_SIZE);
        
        // Set HAL Entry
        bp->hal_entry_addr = elf_header->elf_entry;
        bp->hal_vaddr_end = vaddr_end;
        
        lprintf("\tHAL entry @ %lx, vaddr end @ %lx\n", bp->hal_entry_addr, vaddr_end);
    }
    
    // We just loaded kernel
    else if (type == LOAD_KERNEL) {
        bp->kernel_entry_addr = elf_header->elf_entry;
        lprintf("\tKernel entry @ %lx\n", bp->kernel_entry_addr);
    }
    
    // Can't be this
    else {
        lprintf("\tWhat did I load?\n");
        panic();
    }
}


/*
 * It's possible that paging has not yet been completely set up at this point.
 * Two arguments offset_up and offset_down are used to adjust the actual
 * loading virtual address of the images. On most architectures, they should
 * be zero.
 */
void load_images(struct coreimg_fat *coreimg, struct boot_parameters *bp,
                 ulong offset_up, ulong offset_down)
{
    find_and_layout(coreimg, bp, "tdlrhal.bin", LOAD_HAL, offset_up, offset_down);
    find_and_layout(coreimg, bp, "tdlrkrnl.bin", LOAD_KERNEL, offset_up, offset_down);
}
