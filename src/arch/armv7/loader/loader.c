#include "periph.h"
#include "common/include/memory.h"
#include "common/include/bootparam.h"
#include "common/include/coreimg.h"
#include "common/include/elf32.h"


/*
 * Helper functions
 */
static void panic()
{
    lprintf("Panic!");
    
    while (1) {
        led_blink(100, 100);
    }
}

static int compare(char *src, char *dest, int len)
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
    boot_param.ap_l1table_ptr = 0;
    boot_param.ap_stack_top_ptr = 0;
    
    // HAL & kernel
    boot_param.hal_start_flag = 0;
    boot_param.hal_entry_addr = 0;
    boot_param.hal_vaddr_end = 0;
    boot_param.hal_vspace_end = 0;
    boot_param.kernel_entry_addr = 0;
    
    // Memory map
    boot_param.free_addr_start = 2 * 1024 * 1024;
    boot_param.mem_size = (u64)0x40000000ull;
    
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
    
    // Usable memory (2MB to periph base)
    boot_param.mem_zones[zone_count].start_paddr = 0x100000;
    boot_param.mem_zones[zone_count].len = PERIPHERAL_BASE - 0x200000;
    boot_param.mem_zones[zone_count].type = 1;
    zone_count++;
    
    boot_param.mem_zone_count = zone_count;
    
    lprintf("Done!\n");
}


/*
 * Paging
 */
#define PAGE_TABLE_BASE     0x17C000
#define SDRAM_START         0x0
#define SDRAM_END           0x40000000 //PERIPHERAL_BASE
#define CACHE_DISABLED      0x12
#define CACHE_WRITEBACK     0x1e

static void setup_paging()
{
    static volatile u32 *page_table = (volatile u32 *)PAGE_TABLE_BASE;
    u32 i;
    u32 reg;
    
    lprintf("Setup paging @ %x ... ", (u32)page_table);

    // Set up an 1 to 1 mapping for all 4GB, rw for everyone
    for (i = 0; i < 4096; i++) {
        page_table[i] = (i << 20) | (3 << 10) | CACHE_DISABLED;
    }
    
    // Enable cacheable and bufferable for RAM only
    for (i = SDRAM_START >> 20; i < (SDRAM_END >> 20); i++) {
        page_table[i] = (i << 20) | (3 << 10) | CACHE_WRITEBACK;
    }
    
    // Map the highest 1MB (HAL + kernel) to physical 1MB to 2MB
    page_table[4095] = (0x1 << 20) | (3 << 10) | CACHE_WRITEBACK;

    // Copy the page table address to cp15
    __asm__ __volatile__ (
        "mcr p15, 0, %0, c2, c0, 0;"
        :
        : "r" (page_table)
        : "memory"
    );
    
    // Set the access control to all-supervisor
    __asm__ __volatile__ (
        "mcr p15, 0, %0, c3, c0, 0"
        :
        : "r" (~0x0)
    );

    // Enable the MMU
    __asm__ __volatile__ (
        "mrc p15, 0, %0, c1, c0, 0"
        : "=r" (reg)
        :
        : "cc"
    );
    
    reg |= 0x1;
    
    __asm__ __volatile__ (
        "mcr p15, 0, %0, c1, c0, 0"
        :
        : "r" (reg)
        : "cc"
    );
    
    lprintf("Done!\n");
}


/*
 * Loading images
 */
extern int __end;

static struct coreimg_fat *coreimg;

static void init_coreimage()
{
    coreimg = (struct coreimg_fat *)&__end;
}

static void find_and_layout(char *name, int bin_type)
{
    u32 i;
    
    // Find the file
    struct coreimg_record *record = NULL;
    int found = 0;
    
    for (i = 0; i < coreimg->header.file_count; i++) {
        record = &coreimg->records[i];
        
        if (!compare((u8 *)name, record->file_name, 20)) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
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
    hal_entry = (void *)boot_param.hal_entry_addr;
    hal_entry();
}


void main(unsigned long r0, unsigned long r1, unsigned long atags)
{
    init_bss();
    init_periph();
    
    lprintf("Welcome to Toddler!\n");
    lprintf("Loader arguments r0: %x, r1: %x, atags: %x\n", r0, r1, atags);
    
    build_bootparam();
    setup_paging();
    
    panic();
    
    init_coreimage();
    find_and_layout("tdlrhal.bin", 1);
    find_and_layout("tdlrkrnl.bin", 2);
    
    jump_to_hal();
    
    while (1) {
        lprintf("Should never arrive here!\n");
        panic();
    }
}
