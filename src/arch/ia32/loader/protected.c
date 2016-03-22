asm ("jmp main");


#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/coreimg.h"
#include "common/include/bootparam.h"
#include "common/include/memory.h"
#include "common/include/elf32.h"

#include "loader/loader.h"
#include "loader/font.h"


static struct loader_variables *loader_var;
static struct boot_parameters *boot_param;


static void no_inline memcpy(void* src, void* dest, u32 length)
{
    u8* current_src = (u8*)src;
    u8* current_dest = (u8*)dest;
    
    u32 i;
    
    for (i = 0; i < length; i++) {
        current_dest[i] = current_src[i];
    }
}

static void no_inline set_cursor_pos(u32 line, u32 column)
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
    );
}

static void no_inline print_new_line()
{
    boot_param->cursor_row++;
    boot_param->cursor_col = 0;
    
    if (boot_param->video_mode == 0) {
        set_cursor_pos(boot_param->cursor_row, boot_param->cursor_col);
    }
}

static void no_inline draw_char(u8 ch, u32 line, u32 col)
{
    u32 bytes = boot_param->bytes_per_line;
    u32 bits = boot_param->bits_per_pixel;
    u32 offset = 16 * line * bytes + 8 * col * bits / 8;
    u32 offset_x;
    
    u8 *font = vga_font[ch < 0x20 ? 0 : ch - 0x20];
    u8 cur_map;
    u8 *fb = (u8 *)boot_param->framebuffer_addr;
    
    int i, j;
    for (i = 0; i < 16; i++) {
        cur_map = font[i];
        offset_x = offset;
        for (j = 0; j < 8; j++) {
            if (cur_map & (0x1 << (7 - j))) {
                fb[offset_x + 0] = 0xc0;
                fb[offset_x + 1] = 0xc0;
                fb[offset_x + 2] = 0xc0;
            } else {
                fb[offset_x + 0] = 0;
                fb[offset_x + 1] = 0;
                fb[offset_x + 2] = 0;
            }
            
            offset_x += bits / 8;
        }
        
        offset += bytes;
    }
}

static void no_inline print_char_px(char ch)
{
    draw_char(ch, boot_param->cursor_row, boot_param->cursor_col);
    
    boot_param->cursor_col++;
    if (boot_param->res_x / 8 == boot_param->cursor_col) {
        print_new_line();
    }
}

static void no_inline print_char_tx(char ch)
{
    u32 position = (boot_param->cursor_row * 80 + boot_param->cursor_col) * 2;
    
    __asm__ __volatile__
    (
        "movb   $0x7, %%ah;"
        "movw   %%ax, %%ds:(%%edi)"
        :
        : "D" (TEXT_VIDEO_MEM_ADDR + position), "a" (ch)
    );
    
    boot_param->cursor_col++;
    if (80 == boot_param->cursor_col) {
        print_new_line();
    } else {
        set_cursor_pos(boot_param->cursor_row, boot_param->cursor_col);
    }
}

static void no_inline print_char(char ch)
{
    if (boot_param->video_mode == 0) {
        print_char_tx(ch);
    } else {
        print_char_px(ch);
    }
}

static void no_inline print_string(char *str)
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
            boot_param->cursor_col /= 8;
            boot_param->cursor_col = (boot_param->cursor_col + 1) * 8;
            if (boot_param->cursor_col >= 80) {
                print_new_line();
            } else if (boot_param->video_mode == 0) {
                set_cursor_pos(boot_param->cursor_row, boot_param->cursor_col);
            }
            break;
        default:
            print_char(*str);
            break;
        }
        
        str++;
    }
}

// static void no_inline print_bin(u32 n)
// {
//     u32 i, value;
//     
//     for (i = 0; i < sizeof(u32) * 8; i++) {
//         value = n;
//         value = value << i;
//         value = value >> sizeof(u32) * 8 - 1;
//         
//         print_char(value ? '1' : '0');
//     }
//     
//     print_char('b');
// }

static void real_mode print_dec(u32 n)
{
    int divider = 1000000000;
    int started = 0;
    u32 num = n;
    int i;
    
    if (!n) {
        print_char('0');
    }
    
    for (i = 0; i < 10; i++) {
        if (num / divider) {
            started = 1;
            print_char('0' + num / divider);
        } else if (started) {
            print_char('0');
        }
        
        num %= divider;
        divider /= 10;
    }
}

static void no_inline print_hex(u32 n)
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

static void no_inline stop()
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

static void no_inline print_done()
{
    print_string(" Done!\n");
}

static void no_inline print_failed()
{
    print_string(" Failed!\n");
    stop();
}

static void no_inline enter_protected_mode()
{
    print_string("We are now in protected mode!\n");
}

static void no_inline setup_video()
{
    print_string("Video Mode: ");
    print_dec(boot_param->res_x);
    print_string("x");
    print_dec(boot_param->res_y);
    print_string(" @ ");
    print_dec(boot_param->bits_per_pixel);
    print_string(", FB Address: ");
    print_hex(boot_param->framebuffer_addr);
    print_string(", Bytes per Line: ");
    print_dec(boot_param->bytes_per_line);
    print_new_line();
}

static void no_inline init_hardware()
{
    print_string("Initializing Hardwares ...");
    print_done();
}

static void no_inline build_bootparam()
{
    print_string("Building Boot Parameters ...");
    
    // Memory map
    u64 memory_size = 0;
    u32 i, j;
    u64 current_base_address = 0;
    u64 current_length = 0xffffffff;
    u32 current_type = 0;
    u32 zone_count = 0;
    print_char('|');
    
    do {
        // Find the first block 
        u64 current_base_address_find = -1;
        u32 first_block_index = 0xffffffff;
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
        
        // Did not find a block? We have got the full memory map!
        if (0xffffffff == first_block_index) {
            break;
        }
        
        // Then we combine all contiguious blocks
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
        
        // Calculate memory size
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
        
        // Add to boot param list
        if (
            1 == current_type       // usable
            //|| 3 == current_type  // ACPI reclaimable
        ) {
            boot_param->mem_zones[zone_count].start_paddr = current_base_address;
            boot_param->mem_zones[zone_count].len = current_length;
            boot_param->mem_zones[zone_count].type = current_type;
            boot_param->mem_zone_count = ++zone_count;
        }
        
        // Move to next block
        current_base_address = current_base_address + current_length;
        
        print_char('.');
    } while (1);
    
//     // Free PFN start
//     if (boot_param->free_pfn_start) {
//         boot_param->free_pfn_start++;
//     } else {
//         boot_param->free_pfn_start = KERNEL_INIT_PTE_START_PFN;
//     }
//     print_string("|.");
    
    // Memory size
    if (memory_size % (1024 * 1024)) {
        memory_size = memory_size >> 20;
        memory_size++;
        memory_size = memory_size << 20;
    }
    boot_param->mem_size = memory_size;
    print_string("|.");
    
    // Loader functions
    boot_param->loader_func_type_ptr = loader_var->loader_func_type_ptr;
    boot_param->ap_entry_addr = loader_var->ap_entry_addr;
    boot_param->bios_invoker_addr = loader_var->bios_invoker_addr;
    boot_param->ap_page_dir_pfn_ptr = loader_var->ap_page_dir_pfn_ptr;
    boot_param->ap_stack_top_ptr = loader_var->ap_stack_top_ptr;
    print_string("|.");
    
    // HAL start flag
    boot_param->hal_start_flag = 0;
    print_string("|.");
    
    print_done();
}

static u32 no_opt no_inline page_alloc()
{
    // Init boot param
    if (!boot_param->free_pfn_start) {
        boot_param->free_pfn_start = KERNEL_INIT_PTE_START_PFN;
    }
    
    u32 result = boot_param->free_pfn_start;
    boot_param->free_pfn_start++;

    // Zero the page
    struct page_frame *pg = (struct page_frame *)PFN_TO_ADDR(result);
    u32 i;
    for (i = 0; i < 1024; i++) {
        pg->value_u32[i] = 0;
    }
    
    return result;
}

static void no_opt no_inline setup_paging()
{
    print_string("Setting up Paging ...");
    
    // Get the page directory
    struct page_frame *dir = (struct page_frame *)KERNEL_PDE_PADDR;
    
    u32 i;
    for (i = 1; i < 1023; i++) {
        dir->value_u32[i] = 0;
    }
    
    // First and last entry in page directory should not be empty
    dir->value_pde[0].pfn = KERNEL_PTE_LO4_PFN;
    dir->value_pde[0].present = 1;
    dir->value_pde[0].rw = 1;
    
    dir->value_pde[1023].pfn = KERNEL_PTE_HI4_PFN;
    dir->value_pde[1023].present = 1;
    dir->value_pde[1023].rw = 1;
    
    /*
     * Page table for low 4MB: Direct map.
     */
    struct page_frame *pg = (struct page_frame *)KERNEL_PTE_LO4_PADDR;
    
    // Regular direct map
    for (i = 0; i < 1024; i++) {
        pg->value_u32[i] = 0;
        pg->value_pte[i].pfn = i;
        pg->value_pte[i].present = 1;
        pg->value_pte[i].rw = 1;
    }
    
    // 0xa0000 to 0xfffff should not be cached
    for (i = 160; i < 256; i++) {
        pg->value_pte[i].cache_disabled = 1;
    }
    
    
    /*
     * Page table for high 4MB
     */
    pg = (struct page_frame *)KERNEL_PTE_HI4_PADDR;
    
    
    // Zero the page
    for (i = 0; i < 1024; i++) {
        pg->value_u32[i] = 0;
    }
    
    /*
     * Page table for HAL
     *  The highest 512KB is mapped to HAL loading area
     */
    for (i = 896; i < 1024; i++) {
        pg->value_u32[i] = 0;
        pg->value_pte[i].pfn = HAL_EXEC_START_PFN + i - 896;
        pg->value_pte[i].present = 1;
        pg->value_pte[i].rw = 1;
    }
    
    /*
     * Page table for Kernel
     *  The 2nd highest 512KB is mapped to kernel loading area
     */
    pg = (struct page_frame *)KERNEL_PTE_HI4_PADDR;
    for (i = 768; i < 896; i++) {
        pg->value_u32[i] = 0;
        pg->value_pte[i].pfn = KERNEL_EXEC_START_PFN + i - 768;
        pg->value_pte[i].present = 1;
        pg->value_pte[i].rw = 1;
    }
    
    /*
     * Map vieo framebuffer
     */
    u32 fb_start = boot_param->framebuffer_addr;
    u32 fb_end = boot_param->framebuffer_addr + boot_param->bytes_per_line * boot_param->res_y;
    
    // Round down fb_start
    fb_start /= PAGE_SIZE;
    fb_start *= PAGE_SIZE;
    
    // Round up fb_end
    if (fb_end % PAGE_SIZE) {
        fb_end /= PAGE_SIZE;
        fb_end++;
        fb_end *= PAGE_SIZE;
    }
    
    boot_param->free_pfn_start = KERNEL_INIT_PTE_START_PFN;
    
    u32 fb;
    for (fb = fb_start; fb < fb_end; fb += PAGE_SIZE) {
        u32 pde_index = GET_PDE_INDEX(fb);
        u32 pte_index = GET_PTE_INDEX(fb);
        
        if (!dir->value_u32[pde_index]) {
            dir->value_pde[pde_index].pfn = page_alloc();
            dir->value_pde[pde_index].present = 1;
            dir->value_pde[pde_index].rw = 1;
            dir->value_pde[pte_index].cache_disabled = 1;
        }
        
        pg = (struct page_frame *)(PFN_TO_ADDR(dir->value_pde[pde_index].pfn));
        pg->value_u32[pte_index] = 0;
        pg->value_pte[pte_index].pfn = ADDR_TO_PFN(fb);
        pg->value_pte[pte_index].present = 1;
        pg->value_pte[pte_index].rw = 1;
        pg->value_pte[pte_index].cache_disabled = 1;
    }
    
    /*
     * HAL virtual space end
     */
    boot_param->hal_vspace_end = HAL_SPACE_END_VADDR;
    
    /*
     * Enable paging
     */
    __asm__ __volatile__
    (
        "movl   %%eax, %%cr3;"
        "movl   %%cr0, %%eax;"
        "orl    $0x80000000, %%eax;"
        "movl   %%eax, %%cr0;"
        "jmp   _enable_;"
        "_enable_: nop"
        :
        : "a" (KERNEL_PDE_PADDR)
    );
    
    print_done();
}

static int no_inline compare(u8* src, u8* dest, u8 length)
{
    u32 i;
    
    for (i = 0; i < length; i++) {
        if (src[i] != dest[i]) {
            return 1;
        }
        
        if (src[i] == 0) {
            return 0;
        }
    }
    
    return 0;
}

static void no_inline find_and_layout(char *name, int bin_type)
{
    u32 i;
    
    print_string("Expanding file: ");
    print_string(name);
    print_string(" ...");
    
    // Find the file
    struct coreimg_fat *fat = (struct coreimg_fat *)COREIMG_LOAD_PADDR;
    struct coreimg_record *record = 0;
    int found = 0;
    
    for (i = 0; i < fat->header.file_count; i++) {
        record = &fat->records[i];
        
        if (!compare((u8 *)name, record->file_name, 20)) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        print_failed();
    }
    
    struct elf32_elf_header *elf_header = (struct elf32_elf_header *)(record->start_offset + COREIMG_LOAD_PADDR);
    struct elf32_program_header *header;
    ulong vaddr_end = 0;
    
    /* For every segment, map and load them */
    for (i = 0; i < elf_header->elf_phnum; i++) {
        /* Get header */
        header = (struct elf32_program_header *)((u32)elf_header + elf_header->elf_phoff + elf_header->elf_phentsize * i);
        
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
        if (header->program_vaddr + header->program_memsz > vaddr_end) {
            vaddr_end = header->program_vaddr + header->program_memsz;
        }
        
        print_char('.');
    }
    
    //u32 i;
    //u8* cur_char = (u8*)0x2ffc0;    //0xFFC16f80;
    //for (i = 0; i < 1024; i++) {
    //print_char(*cur_char++);
    //}
    
    //stop();
    
    // We just loaded HAL
    if (bin_type == 1) {
        // HAL Virtual Address End: Align to 4KB
        if (vaddr_end % PAGE_SIZE) {
            vaddr_end /= PAGE_SIZE;
            vaddr_end++;
            vaddr_end *= PAGE_SIZE;
        }
        
        // Set HAL Entry
        *((u32 *)loader_var->hal_entry_addr_ptr) = elf_header->elf_entry;
        boot_param->hal_entry_addr = elf_header->elf_entry;
        boot_param->hal_vaddr_end = vaddr_end;
    }
    
    // We just loaded kernel
    else if (bin_type == 2) {
        boot_param->kernel_entry_addr = elf_header->elf_entry;
    }
    
    print_done();
}

static void no_inline jump_to_hal()
{
    print_string("Starting HAL ... ");
    
    // Setup HAL init stack
    u32 *stack_top = (u32 *)boot_param->ap_stack_top_ptr;
    *stack_top = INIT_STACK_TOP_VADDR;
    
    // Jump back to assembly, then the assembly code will jump to HAL
    __asm__ __volatile__
    (
        "jmp    *%%ebx"
        :
        : "b"(loader_var->return_addr), "D" (boot_param->hal_entry_addr)
    );
}

int main()
{
    loader_var = (struct loader_variables *)LOADER_VARIABLES_PADDR;
    boot_param = (struct boot_parameters *)BOOT_PARAM_PADDR;
    
    enter_protected_mode();
    setup_video();
    init_hardware();
    build_bootparam();
    setup_paging();
    find_and_layout("tdlrhal.bin", 1);
    find_and_layout("tdlrkrnl.bin", 2);
    jump_to_hal();
    
    stop();
    return 0;
}
