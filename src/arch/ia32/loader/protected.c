asm ("jmp main");


#include "loader/loader.h"
#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/coreimg.h"
#include "common/include/bootparam.h"
#include "common/include/elf32.h"
#include "hal/include/init.h"


static struct loader_variables *loader_var;


void set_cursor_pos(u32 line, u32 column)
{
    u32 position = line * 80 + column;
    
    __asm__ __volatile__
    (
        "xorl   %%eax, %%eax;"
        
        "movb   $0xf, %%al;"
        "movw   $0x3d4, %%dx;"          /* Port 0x3d4 */
        "outb   %%al, %%dx;"
        
        "movw   %%cx, %%ax;"
        "movw   $0x3d5, %%dx;"          /* Port 0x3d5 */
        "outb   %%al, %%dx;"            /* Low part of cursor position */
        
        "movb   $0xe, %%al;"
        "movw   $0x3d4, %%dx;"          /* Port 0x3d4 */
        "outb   %%al, %%dx;"
        
        "movw   %%cx, %%ax;"
        "movb   %%ah, %%al;"
        "movw   $0x3d5, %%dx;"          /* Port 0x3d5 */
        "outb   %%al, %%dx"            /* Low part of cursor position */
        :
        : "c" (position)
        : "%%eax", "%%ecx", "%%edx"
    );
}

void print_new_line()
{
    loader_var->cursor_row++;
    loader_var->cursor_col = 0;
    set_cursor_pos(loader_var->cursor_row, loader_var->cursor_col);
}

void print_char(char ch)
{
    /* Calculate cursor position */
    u32 position = (loader_var->cursor_row * 80 + loader_var->cursor_col) * 2;
    
    __asm__ __volatile__
    (
        "movb   $0x7, %%ah;"
        "movw   %%ax, %%ds:(%%edi)"
        :
        : "D" (LOADER_VIDEO_MEMORY_ADDRESS + position), "a" (ch)
    );
    
    loader_var->cursor_col++;
    if (80 == loader_var->cursor_col) {
        print_new_line();
    } else {
        set_cursor_pos(loader_var->cursor_row, loader_var->cursor_col);
    }
}

void print_string(char *str)
{
    while (*str) {
        switch (*str) {
        case '\n':
        case '\r':
            print_new_line();
            break;
        case '\\':
            print_char('\\');
            break;
        case '\t':
            loader_var->cursor_col /= 8;
            loader_var->cursor_col = (loader_var->cursor_col + 1) * 8;
            if (loader_var->cursor_col >= 80) {
                print_new_line();
            } else {
                set_cursor_pos(loader_var->cursor_row, loader_var->cursor_col);
            }
            break;
        default:
            print_char(*str);
            break;
        }
        
        str++;
    }
}

void print_bin(u32 n)
{
    u32 i, value;
    
    for (i = 0; i < sizeof(u32) * 8; i++) {
        value = n;
        value = value << i;
        value = value >> sizeof(u32) * 8 - 1;
        
        print_char(value ? '1' : '0');
    }
    
    print_char('b');
}

void print_dec(u32 n)
{
    u32 i;
    u8 s[12];
    
    for (i = 11; i >= 0; i--) {
        s[i] = 0;
    }
    
    i=0;
    do  {
        s[i++] = n % 10 + '0';
    } while ((n /= 10 ) >  0);
    
    for (i = 11; i >= 0; i--) {
        if (s[i]) {
            print_char(s[i]);
        }
    }
}

void print_hex(u32 n)
{
    u32 i, value;
    
    for (i = 0; i < sizeof(u32) * 8; i += 4) {
        value = n;
        value = value << i;
        value = value >> sizeof(u32) * 7;
        
        print_char(value > 9 ? (u8)(value + 0x30 + 0x27) : (u8)(value + 0x30));
    }
    
    print_char('h');
}

void stop()
{
    print_string("\nUnable to start Toddler!\n");
    
    do {
        __asm__ __volatile__
        (
            "hlt"
            :
            :
        );
    } while (1);
}

void print_done()
{
    print_string(" Done!\n");
}

void print_failed()
{
    print_string(" Failed!\n");
    stop();
}

void memcpy(void* src, void* dest, u32 length)
{
    u8* current_src = (u8*)src;
    u8* current_dest = (u8*)dest;
    
    u32 i;
    
    for (i = 0; i < length; i++) {
        current_dest[i] = current_src[i];
    }
}

void enter_protected_mode()
{
    print_done();
}

void initialize_hardware()
{
    print_string("Initializing Hardwares ...");
    print_done();
}

void build_bootparam()
{
    print_string("Building Boot Parameters ...");
    
    struct boot_param *bp = (struct boot_param *)BOOT_PARAM_PADDR;
    
    // Video
    bp->video_mode = 0;
    print_string("|.");
    
    // Memory map
    u64 memory_size = 0;
    u32 i, j;
    u64 current_base_address = 0;
    u64 current_length = 0xffffffff;
    u32 current_type = 0;
    u64 previous_end_address = 0;
    u32 previous_type = 0;
    u32 zone_count = 0;
    print_char('|');
    
    do {
        /* We find the first block */
        u64 current_base_address_find = -1;
        u32 first_block_index = 0xffffffff;
        previous_end_address = current_base_address;
        previous_type = current_type;
        current_length = 0xffffffff;
        current_type = 0xffffffff;
        
        for (i = 0; i < loader_var->mem_part_count; i++) {
            if (
                loader_var->e820_map[i].base_addr >= current_base_address &&
                loader_var->e820_map[i].base_addr < current_base_address_find
            ) {
                current_base_address_find = loader_var->e820_map[i].base_addr;
                first_block_index = i;
                current_length = loader_var->e820_map[i].len;
                current_type = loader_var->e820_map[i].type;
            }
        }
        
        /* Did not find a block? We have got the full memory map! */
        if (0xffffffff == first_block_index) {
            break;
        }
        
        /* Then we combine all contiguious blocks */
        current_base_address = current_base_address_find;
        for (i = 0; i < loader_var->mem_part_count; i++) {
            for (j = 0; j < loader_var->mem_part_count; j++) {
                if (
                    loader_var->e820_map[j].base_addr >= current_base_address &&
                    loader_var->e820_map[j].base_addr <= current_base_address + current_length &&
                    loader_var->e820_map[j].base_addr + loader_var->e820_map[j].len > current_base_address + current_length &&
                    loader_var->e820_map[j].type == current_type
                ) {
                    current_length = loader_var->e820_map[j].base_addr + loader_var->e820_map[j].len - current_base_address;
                }
            }
        }
        
        /* Calculate memory size */
        if (1 == current_type || 3 == current_type) {
            if (current_base_address < 1024 * 1024) {
                if (current_base_address + current_length < 1024 * 1024) {
                    memory_size = 1024 * 1024;
                } else {
                    memory_size = current_base_address + current_length;
                }
            } else {
                memory_size += current_length;
            }
        }
        
        /* Add to boot param list */
        if (1 == current_type || 3 == current_type) {
            bp->mem_zones[zone_count].start_paddr = current_base_address;
            bp->mem_zones[zone_count].len = current_length;
            bp->mem_zones[zone_count].type = current_type;
            bp->mem_zone_count = ++zone_count;
        }
        
        /* Move to next block */
        current_base_address = current_base_address + current_length;
        
        print_char('.');
    } while (1);
    
    // Memory size
    if (memory_size % (1024 * 1024)) {
        memory_size = memory_size >> 20;
        memory_size++;
        memory_size = memory_size << 20;
    }
    bp->mem_size = memory_size;
    print_string("|.");
    
    // Loader functions
    bp->what_to_load_addr = loader_var->what_to_load_addr;
    bp->ap_entry_addr = loader_var->ap_entry_addr;
    bp->ap_page_dir_pfn_addr = loader_var->ap_page_dir_pfn_addr;
    print_string("|.");
    
    print_done();
}

void setup_paging()
{
    print_string("Setting up Paging ...");
    
    /* Get the page directory */
    s_page_frame* page_frame = (s_page_frame*)HAL_PDE_ADDRESS_PHYSICAL;
    
    u32 i, j;
    for (i = 1; i < 1023; i++) {
        page_frame->value_u32[i] = 0;       /* Empty entries */
    }
    
    /* First and last entry in page directory should not be empty */
    page_frame->value_pde[0].pfn = HAL_PTE_LOW_4MB_PFN;
    page_frame->value_pde[0].present = 1;
    page_frame->value_pde[0].read_write = 1;
    page_frame->value_pde[0].user_supervisor = 0;                           /* Ring 0: Supervisor */
    
    page_frame->value_pde[1023].pfn = HAL_PTE_HIGH_4MB_PFN;
    page_frame->value_pde[1023].present = 1;
    page_frame->value_pde[1023].read_write = 1;
    page_frame->value_pde[1023].user_supervisor = 0;                        /* Ring 0: Supervisor */
    
    /*
     * Page table for low 4MB: Direct map.
     * But the first page should be reserved so that a null pointer could be detected easily.
     * Note: Actually we map the first page, because it is required by BIOS Invoker
     */
    page_frame = (s_page_frame*)HAL_PTE_LOW_4MB_ADDRESS_PHYSICAL;
    page_frame->value_u32[0] = 0;
    for (i = 0; i < 1024; i++) {
        page_frame->value_pte[i].pfn = i;
        page_frame->value_pte[i].present = 1;
        page_frame->value_pte[i].read_write = 1;
        page_frame->value_pte[i].user_supervisor = 0;                   /* Ring 0: Supervisor */
    }
    //page_frame->value_u32[0] = 0;
    
    /*
     * Page table for high 4MB: Last 4MB <-> HAL Loading Location.
     * But the last page should be reserved so that a some errors could be detected easily.
     */
    page_frame = (s_page_frame*)HAL_PTE_HIGH_4MB_ADDRESS_PHYSICAL;
    for (i = 0; i < 1024; i++) {
        page_frame->value_pte[i].pfn = HAL_EXECUTE_START_PFN + i;
        page_frame->value_pte[i].present = 1;
        page_frame->value_pte[i].read_write = 1;
        page_frame->value_pte[i].user_supervisor = 1;                   /* Ring 0: Supervisor */
    }
    page_frame->value_u32[1023] = 0;
    
    /* Enable paging */
    __asm__ __volatile__
    (
        "movl   %%eax, %%cr3;"
        "movl   %%cr0, %%eax;"
        "orl    $0x80000000, %%eax;"
        "movl   %%eax, %%cr0;"
        "jmp   _enable_;"
        "_enable_: nop"
        :
        : "a" (HAL_PDE_PFN << 12)
    );
    
    /* Zero memory at the highest 4MB */
    for (i = 0; i < 1023; i++) {
        for (j = 0; j < 1024; j++) {
            *((u32*)(HAL_VIRTUAL_MEMORY_START_ADDRESS + i * j)) = 0;
        }
    }
    
    print_done();
}

u32 layout_hal()
{
    print_string("Expanding HAL ...");
    
    print_new_line();
    
    //stop();
    struct coreimg_record *hal_file_record = (struct coreimg_record *)(HAL_KRNLIMG_LOADED_BY_LOADER_ADDRESS_PHYSICAL + 32);
    struct elf32_elf_header *elf_header = (struct elf32_elf_header *)(hal_file_record->start_offset + HAL_KRNLIMG_LOADED_BY_LOADER_ADDRESS_PHYSICAL);
    struct s_elf32_program_header *header;
    
    loader_var->hal_vaddr_end = 0;
    
    /* For every segment, map and load them */
    u32 i;
    for (i = 0; i < elf_header->elf_phnum; i++) {
        /* Get header */
        header = (s_elf32_program_header*)((u32)elf_header + elf_header->elf_phoff + elf_header->elf_phentsize * i);
        
        //print_string("  Program #");
        //print_hex(i);
        
        //print_string(": Offset ");
        //print_hex(header->program_offset);
        
        //print_string(", VAddr ");
        //print_hex(header->program_vaddr);
        
        //print_string(", FileSz ");
        //print_hex(header->program_filesz);
        
        //print_string(", MemSz ");
        //print_hex(header->program_memsz);
        
        //print_new_line();
        
        // Copy the program data
        if (header->program_filesz) {
            memcpy(
                (void*)(header->program_offset + (u32)elf_header),
                   (void*)header->program_vaddr,
                   header->program_filesz
            );
        }
        
        // Get the end of virtual address of HAL
        if (header->program_vaddr + header->program_memsz > loader_var->hal_vaddr_end) {
            loader_var->hal_vaddr_end = header->program_vaddr + header->program_memsz;
        }
        
        print_char('.');
    }
    
    //u32 i;
    //u8* cur_char = (u8*)0x2ffc0;    //0xFFC16f80;
    //for (i = 0; i < 1024; i++) {
    //print_char(*cur_char++);
    //}
    
    //stop();
    
    // Set HAL Entry
    *((u32 *)loader_var->hal_entry_addr) = elf_header->elf_entry;
    
    // HAL Virtual Address End: Align to 4KB
    if (loader_var->hal_vaddr_end % 4096) {
        loader_var->hal_vaddr_end = loader_var->hal_vaddr_end >> 12;
        loader_var->hal_vaddr_end++;
        loader_var->hal_vaddr_end = loader_var->hal_vaddr_end << 12;
    }
    
    print_done();
}

void jump_to_hal()
{
    print_string("Starting HAL ... ");
    
    /* Construct start parameters */
    s_hal_start_param* hal_start_param = (s_hal_start_param*)HAL_START_PARAM_ADDRESS;
    hal_start_param->param_type = 0;
    hal_start_param->cursor_row = loader_var->cursor_row;
    hal_start_param->cursor_col = loader_var->cursor_col;
    hal_start_param->hal_vaddr_end = loader_var->hal_vaddr_end;
    
    /* Jump to the assembly */
    __asm__ __volatile__
    (
        "jmp    *%%ebx"
        :
        : "b"(loader_var->return_address)
    );
}

int main()
{
    loader_var = (s_boot_loader_variables*)(LOADER_ADDRESS_32_BASE + LOADER_VARIABLES_ADDRESS_OFFSET);
    
    enter_protected_mode();
    initialize_hardware();
    build_bootparam();
    setup_paging();
    layout_hal();
    jump_to_hal();
    
    stop();
    return 0;
}
