#include "loader/periph.h"
#include "common/include/bootparam.h"
#include "common/include/coreimg.h"
#include "common/include/memory.h"
#include "common/include/elf32.h"


/*
 * Helper functions
 */
static void panic()
{
    lprintf("Panic!");
    
    while (1) {
        // do nothing
    }
}

static u32 raw_read(u32 addr)
{
    volatile u32 *ptr = (u32 *)addr;
    return *ptr;
}

static void raw_write(u32 addr, u32 val)
{
    volatile u32 *ptr = (u32 *)addr;
    *ptr = val;
}

static int strdiff(char *src, char *dest, int len)
{
    int i;
    
    for (i = 0; i < len; i++) {
        if (src[i] != dest[i]) {
            return 1;
        } else if (src[i] == 0) {
            return 0;
        }
    }
    
    return 0;
}

static int strlen(char *s)
{
    int len = 0;
    
    while (*s++) {
        len++;
    }
    
    return len;
}

static void memcpy(void *dest, void *src, u32 count)
{
    u32 i;
    unsigned char *s = (unsigned char *)src;
    unsigned char *d = (unsigned char *)dest;
    
    for (i = 0; i < count; i++) {
        *(d++) = *(s++);
    }
    
}

static void memzero(void *dest, u32 count)
{
    u32 i;
    unsigned char *d = (unsigned char *)dest;
    
    for (i = 0; i < count; i++) {
        *(d++) = (unsigned char)0x0;
    }
    
}


/*
 * BSS
 */
extern int __bss_start;
extern int __bss_end;

static void init_bss()
{
    int *cur;
    
    for (cur = &__bss_start; cur < &__bss_end; cur++) {
        *cur = 0;
    }
}


/*
 * Bootloader arguments
 */
static u32 coreimg_start_addr = 0;
static u32 coreimg_size = 0;

static u32 memory_size;

static void parse_bootloader_args(u32 kargc, u32 karg_addr, u32 env_addr, u32 ram_size)
{
    // Memory size
    memory_size = ram_size;
    
    // Find out rd_start and rd_size
    u32 addr;
    u32 search_end = karg_addr + kargc * 256;
    
    char *rd_start = NULL;
    char *rd_size = NULL;
    char ch;
    
    for (addr = karg_addr; addr <= search_end; addr += 4) {
        // rd_s -> 0x735f6472, tart -> 0x74726174
        if (raw_read(addr) == 0x735f6472 && raw_read(addr + 4) == 0x74726174) {
            rd_start = (char *)(addr + 0x8 + 0x3);
            if (strlen(rd_start) == 16) {
                rd_start += 0x8;
            }
        }
        
        // rd_s -> 0x735f6472, ize= -> 0x3d657a69
        else if (raw_read(addr) == 0x735f6472 && raw_read(addr + 4) == 0x3d657a69) {
            rd_size = (char *)(addr + 0x8);
        }
    }
    
    lprintf("Found rd_start @ %x, rd_size @ %x\n", rd_start, rd_size);
    
    // Convert the strings to integer
    ch = *rd_start;
    while (ch && ch != ' ' && ch != '\t') {
        coreimg_start_addr <<= 4;
        
        if (ch >= 'a' && ch <= 'f') {
            coreimg_start_addr += ch - 'a' + 0xa;
        } else if (ch >= 'A' && ch <= 'F') {
            coreimg_start_addr += ch - 'A' + 0xa;
        } else {
            coreimg_start_addr += ch - '0';
        }
        
        rd_start++;
        ch = *rd_start;
    }
    
    ch = *rd_size;
    while (ch && ch != ' ' && ch != '\t') {
        coreimg_size *= 10;
        coreimg_size += ch - '0';
        
        rd_size++;
        ch = *rd_size;
    }
    
    // Done
    lprintf("Boot loader arguments parsed, coreimg start: %x, size: %x, memory size: %x\n", coreimg_start_addr, coreimg_size, memory_size);
}


/*
 * Boot parameters
 */
struct boot_parameters boot_param;

static void build_bootparam()
{
    lprintf("Building boot parameters @ %x ... ", (u32)&boot_param);
    
    // Boot info
    boot_param.boot_dev = 0;
    boot_param.boot_dev_info = 0;
    
    // Loader func
    boot_param.loader_func_type_ptr = 0;
    
    // AP starter
    boot_param.ap_entry_addr = 0;
    boot_param.ap_page_table_ptr = 0;
    boot_param.ap_stack_top_ptr = 0;
    
    // HAL & kernel
    boot_param.hal_start_flag = 0;
    boot_param.hal_entry_addr = 0;
    boot_param.hal_vaddr_end = 0;
    boot_param.hal_vspace_end = 0x80180000;
    boot_param.kernel_entry_addr = 0;
    
    // Memory map
    boot_param.free_addr_start = 2 * 1024 * 1024;
    boot_param.free_pfn_start = ADDR_TO_PFN(boot_param.free_addr_start);
    boot_param.mem_size = (u64)memory_size;
    
    // Memory zones
    u32 zone_count = 0;
    
    // Reserved by HW and loader programs (lowest 64KB)
    boot_param.mem_zones[zone_count].start_paddr = 0x0;
    boot_param.mem_zones[zone_count].len = 0x8000;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // Core image (64KB to 1MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x8000;
    boot_param.mem_zones[zone_count].len = 0x100000 - 0x8000;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // HAL and kernel (1MB to 2MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x100000;
    boot_param.mem_zones[zone_count].len = 0x100000;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // Usable memory (2MB to 128MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x200000;
    boot_param.mem_zones[zone_count].len = memory_size - 0x200000;
    boot_param.mem_zones[zone_count].type = 1;
    zone_count++;
    
    boot_param.mem_zone_count = zone_count;
    
    lprintf("Done!\n");
}


/*
 * Load ELF image
 */
static struct coreimg_fat *coreimg;

static void init_coreimage()
{
    coreimg = (struct coreimg_fat *)coreimg_start_addr;
    lprintf("Core image loaded @ %x\n", coreimg);
}

static void find_and_layout(char *name, int bin_type)
{
    u32 i;
    
    // Find the file
    struct coreimg_record *record = NULL;
    int found = 0;
    
    for (i = 0; i < coreimg->header.file_count; i++) {
        record = &coreimg->records[i];
        
        if (!strdiff((u8 *)name, record->file_name, 20)) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        lprintf("Unable to find file: %s\n", name);
        panic();
    }
    
    // Load the file
    struct elf32_elf_header *elf_header = (struct elf32_elf_header *)(record->start_offset + (ulong)coreimg);
    struct elf32_program_header *header;
    ulong vaddr_end = 0;
    
    // For each segment
    for (i = 0; i < elf_header->elf_phnum; i++) {
        header = (struct elf32_program_header *)((u32)elf_header + elf_header->elf_phoff + elf_header->elf_phentsize * i);
        
        // Copy the program data
        if (header->program_filesz) {
            lprintf("\tCopy section: %x -> %x\n", header->program_offset + (u32)elf_header, header->program_vaddr);
            memcpy(
                (void *)header->program_vaddr,
                (void *)(header->program_offset + (u32)elf_header),
                header->program_filesz
            );
        }
        
        // Zero the memory if necessary
        else if (header->program_memsz) {
            memzero((void *)header->program_vaddr, header->program_memsz);
        }
        
        // Get the end of virtual address
        if (header->program_vaddr + header->program_memsz > vaddr_end) {
            vaddr_end = header->program_vaddr + header->program_memsz;
        }
    }
    
    // We just loaded HAL
    if (bin_type == 1) {
        // HAL Virtual Address End: Align to page size
        if (vaddr_end % PAGE_SIZE) {
            vaddr_end /= PAGE_SIZE;
            vaddr_end++;
            vaddr_end *= PAGE_SIZE;
        }
        
        // Set HAL Entry
        boot_param.hal_entry_addr = elf_header->elf_entry;
        boot_param.hal_vaddr_end = vaddr_end;
    }
    
    // We just loaded kernel
    else if (bin_type == 2) {
        boot_param.kernel_entry_addr = elf_header->elf_entry;
    }
}


/*
 * Jump to HAL!
 */
static void (*hal_entry)();

static void jump_to_hal()
{
    lprintf("Starting HAL ... ");
    
    hal_entry = (void *)boot_param.hal_entry_addr;
    hal_entry(&boot_param);
}


/*
 * Main
 */
void main(u32 kargc, u32 karg_addr, u32 env_addr, u32 ram_size)
{
    init_bss();
    init_periph();
    
    lprintf("Args: %x, %x, %x, %x\n", kargc, karg_addr, env_addr, ram_size);
    
    lprintf("Welcome to Toddler!\n");
    
    parse_bootloader_args(kargc, karg_addr, env_addr, ram_size);
    build_bootparam();
    
    init_coreimage();
    find_and_layout("tdlrhal.bin", 1);
    find_and_layout("tdlrkrnl.bin", 2);
    
    jump_to_hal();
    
    while (1) {
        lprintf("Should never arrive here!\n");
        panic();
    }
}
