#include "loader/periph.h"
#include "common/include/asm.h"
#include "common/include/reg.h"
#include "common/include/bootparam.h"
#include "common/include/coreimg.h"
#include "common/include/memory.h"
#include "common/include/elf32.h"


/*
 * Helper functions
 */
#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))

static void panic()
{
    lprintf("Panic!");
    while (1);
}

static u32 raw_read(ulong addr)
{
    volatile u32 *ptr = (u32 *)addr;
    return *ptr;
}

static void raw_write(ulong addr, u32 val)
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

static void memcpy(void *dest, void *src, ulong count)
{
    ulong i;
    unsigned char *s = (unsigned char *)src;
    unsigned char *d = (unsigned char *)dest;
    
    for (i = 0; i < count; i++) {
        *(d++) = *(s++);
    }
    
}

static void memzero(void *dest, ulong count)
{
    ulong i;
    unsigned char *d = (unsigned char *)dest;
    
    for (i = 0; i < count; i++) {
        *(d++) = (unsigned char)0x0;
    }
    
}

static u32 swap_endian32(u32 val)
{
    u32 rr = val & 0xff;
    u32 rl = (val >> 8) & 0xff;
    u32 lr = (val >> 16) & 0xff;
    u32 ll = (val >> 24) & 0xff;
    
    u32 swap = (rr << 24) | (rl << 16) | (lr << 8) | ll;
    return swap;
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
 * CPU configs
 */
static void init_cpu()
{
    struct cp0_status reg;
    
    // Read CP0_STATUS
    read_cp0_status(reg.value);
    
    // Enable 64-bit segments
#if (ARCH_WIDTH == 64)
    reg.kx = 1;
    reg.sx = 1;
    reg.ux = 1;
    reg.px = 1;
#endif
    
    // Disable interrupts
    reg.ie = 0;
    reg.ksu = 0;
    reg.exl = 0;
    reg.erl = 0;
    
    // Apply to CP0_STATUS
    write_cp0_status(reg.value);
}


/*
 * Bootloader arguments
 */
extern int __end;
static ulong memory_size;

static ulong coreimg_start_addr = 0;
static ulong coreimg_size = 0;
static struct coreimg_fat *coreimg;

static void parse_bootloader_args(ulong kargc, ulong karg_addr, ulong env_addr, ulong ram_size)
{
    lprintf("Parsing boot arguments\n");
    
    // Memory size
    memory_size = ram_size;
    
    // Find out rd_start and rd_size
    ulong addr;
    ulong search_end = karg_addr + kargc * 256;
    
    char *rd_start = NULL;
    char *rd_size = NULL;
    char ch;
    
    for (addr = karg_addr; addr <= search_end; addr += 4) {
        if (!strdiff((char *)addr, "rd_start", 8)) {
            rd_start = (char *)(addr + 0x8 + 0x3);
                if (strlen(rd_start) == 16) {
                    rd_start += 0x8;
                }
        }
        
        else if (!strdiff((char *)addr, "rd_size", 7)) {
            rd_size = (char *)(addr + 0x8);
        }
    }
    
    lprintf("\tFound rd_start @ %lx, rd_size @ %lx\n", rd_start, rd_size);
    
    // Convert the strings to integer
    ulong coreimg_default_addr = 0;
    ch = *rd_start;
    while (ch && ch != ' ' && ch != '\t') {
        coreimg_default_addr <<= 4;
        
        if (ch >= 'a' && ch <= 'f') {
            coreimg_default_addr += ch - 'a' + 0xa;
        } else if (ch >= 'A' && ch <= 'F') {
            coreimg_default_addr += ch - 'A' + 0xa;
        } else {
            coreimg_default_addr += ch - '0';
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
    
    lprintf("\tCoreimg default start addr: %lx, size: %lx, memory size: %lx\n",
            coreimg_default_addr, coreimg_size, memory_size);
    
    // Copy the image to a lower address
    // FIXME: using __end may not be accurate, need to check this again
    coreimg_start_addr = (ulong)&__end;
    coreimg_start_addr = ALIGN_UP(coreimg_start_addr, PAGE_SIZE);
    
    memcpy((void *)coreimg_start_addr, (void *)coreimg_default_addr, coreimg_size);
    coreimg = (struct coreimg_fat *)coreimg_start_addr;
    
    lprintf("\tCore image loaded @ %lx\n", coreimg_start_addr);
}


/*
 * Boot parameters
 */
struct boot_parameters boot_param;

static void build_bootparam()
{
    lprintf("Building boot parameters @ %lx ... ", &boot_param);
    
    // Boot info
    boot_param.boot_dev = 0;
    boot_param.boot_dev_info = 0;
    
    // Loader func
    boot_param.loader_func_type_ptr = 0;
    
    // AP starter
    boot_param.ap_entry_addr = 0;
    boot_param.ap_page_table_ptr = 0;
    boot_param.ap_stack_top_ptr = 0;
    
    // Core image
    boot_param.coreimg_load_addr = coreimg_start_addr;
    
    // HAL & kernel
    boot_param.hal_start_flag = 0;
    boot_param.hal_entry_addr = 0;
    boot_param.hal_vaddr_end = 0;
    boot_param.hal_vspace_end = HAL_VSPACE_END;
    boot_param.kernel_entry_addr = 0;
    
    // Memory map
    boot_param.free_addr_start = 2 * 1024 * 1024;
    boot_param.free_pfn_start = ADDR_TO_PFN(boot_param.free_addr_start);
    boot_param.mem_size = (u64)memory_size;
    
    // Memory zones
    int zone_count = 0;
    
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
static void find_and_layout(char *name, int bin_type)
{
    u32 i;
    
    // Tell user the file name
    lprintf("Loading file: %s\n", name);
    
    // Find the file
    u32 file_count = coreimg->header.file_count;
    
    if (ARCH_BIG_ENDIAN != coreimg->header.big_endian) {
        file_count = swap_endian32(file_count);
    }
    
    struct coreimg_record *record = NULL;
    int found = 0;
    for (i = 0; i < file_count; i++) {
        record = &coreimg->records[i];
        
        if (!strdiff((char *)name, (char *)record->file_name, 20)) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        lprintf("Unable to find file: %s\n", name);
        panic();
    }
    
    //lprintf("Here?\n");
    
    // Load the file
    u32 start_offset = record->start_offset;
    if (ARCH_BIG_ENDIAN != coreimg->header.big_endian) {
        start_offset = swap_endian32(start_offset);
    }
    
//     struct elf32_header *elf_header = (struct elf32_header *)(start_offset + (ulong)coreimg);
//     struct elf32_program *header;
    elf_native_header_t *elf_header = (elf_native_header_t *)((ulong)start_offset + (ulong)coreimg);
    elf_native_program_t *header;
    ulong vaddr_end = 0;
    
    // For each segment
    for (i = 0; i < elf_header->elf_phnum; i++) {
        if (i) {
            header = (void *)((ulong)header + (ulong)elf_header->elf_phentsize);
        } else {
            header = (void *)((ulong)elf_header + (ulong)elf_header->elf_phoff);
        }
        
        // Zero the memory
        if (header->program_memsz) {
            lprintf("\tZero memory @ %lx, size: %lx\n", (ulong)header->program_vaddr,
                    (ulong)header->program_memsz);
            memzero((void *)(ulong)header->program_vaddr, header->program_memsz);
        }
        
        // Copy the program data
        if (header->program_filesz) {
            lprintf("\tCopy section @ %lx -> %lx, size: %lx\n",
                    (ulong)header->program_offset + (ulong)elf_header,
                    (ulong)header->program_vaddr,
                    (ulong)header->program_filesz
            );
            
            memcpy(
                (void *)(ulong)header->program_vaddr,
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
    if (bin_type == 1) {
        // HAL Virtual Address End: Align to page size
        vaddr_end = ALIGN_UP(vaddr_end, PAGE_SIZE);
        
        // Set HAL Entry
        boot_param.hal_entry_addr = elf_header->elf_entry;
        boot_param.hal_vaddr_end = vaddr_end;
        
        lprintf("\tHAL entry @ %lx, vaddr end @ %lx\n", boot_param.hal_entry_addr, vaddr_end);
    }
    
    // We just loaded kernel
    else if (bin_type == 2) {
        boot_param.kernel_entry_addr = elf_header->elf_entry;
        lprintf("\tKernel entry @ %lx\n", boot_param.kernel_entry_addr);
    }
}


/*
 * Jump to HAL!
 */
static void (*hal_entry)();

static void jump_to_hal()
{
    lprintf("Starting HAL ... @ %lx\n", boot_param.hal_entry_addr);
    
    hal_entry = (void *)boot_param.hal_entry_addr;
    hal_entry(&boot_param);
}


/*
 * Main
 */
void loader_entry(ulong kargc, ulong karg_addr, ulong env_addr, ulong ram_size)
{
    init_bss();
    init_cpu();
    init_periph();
    
    lprintf("Args: %lx, %lx, %lx, %lx\n", kargc, karg_addr, env_addr, ram_size);
    lprintf("Welcome to Toddler!\n");
    
    parse_bootloader_args(kargc, karg_addr, env_addr, ram_size);
    build_bootparam();
    
    find_and_layout("tdlrhal.bin", 1);
    find_and_layout("tdlrkrnl.bin", 2);
    
    jump_to_hal();
    
    while (1) {
        lprintf("Should never arrive here!\n");
        panic();
    }
}
