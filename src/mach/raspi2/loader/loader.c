#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "common/include/coreimg.h"
#include "common/include/bootparam.h"
#include "common/include/page.h"
#include "common/include/reg.h"
#include "common/include/elf32.h"
#include "loader/include/print.h"
#include "loader/include/lib.h"
#include "loader/include/cmdline.h"
#include "loader/include/exec.h"
#include "loader/include/periph/bcm2835.h"


#define BCM2835_BASE    0x3f000000ul
#define BCM2835_END     0x3ffffffful


static void stop(ulong val)
{
    if (!val) {
        val = 0xbeef;
    }
    
    while (1) {
        __asm__ __volatile__
        (
            "mov r0, %[reg];"
            :
            : [reg] "r" (val)
        );
    }
}


/*
 * Periph
 */
static int fb_enabled = 0;

void init_periph()
{
    // Init BCM2835
    init_bcm2835(BCM2835_BASE, BCM2835_END);
    
    // Init draw char
    if (bcm2835_is_framebuffer_avail()) {
        void *f;
        int w, h, d, p;
        bcm2835_get_framebuffer_info(&f, &w, &h, &d, &p);
        init_framebuffer_draw(f, w, h, d, p);
        fb_enabled = 1;
    }
}

void draw_char(char ch)
{
    bcm2835_pl011_write(ch);
    if (fb_enabled) {
        framebuffer_draw_char(ch);
    }
}


/*
 * ATAGS
 */
struct atag {
    u32 size;
    u32 tag;
    
    union {
        // 0x54410001
        struct {
            u32 flags;          // bit 0 = read-only
            u32 pagesize;       // systems page size (usually 4k)
            u32 rootdev;        // root device number
        } core;
        
        // 0x54410002
        struct {
            u32 size;           // size of the area
            u32 start;          // physical start address
        } mem;
        
        // 0x54410002
        char cmdline[1];        // minimum size
        
        // 0x54410003
        struct {
            u8 x, y;            // width and height
            u16 page;
            u8 mode, cols;
            u16 ega_bx;
            u8 lines, is_vga;
            u16 points;
        } video_test;
        
        // 0x54420004
        struct {
            u32 flags;          // bit 0 = load, bit 1 = prompt
            u32 size;           // decompressed ramdisk size in _kilo_ bytes
            u32 start;          // starting block of floppy-based RAM disk image
        } ramdisk;
        
        // 0x54420005
        struct {
            u32 start;          // physical start address
            u32 size;           // size of compressed ramdisk image in bytes
        } initrd2;
        
        // 0x54420006
        struct {
            u32 low;
            u32 high;
        } serial;
        
        // 0x54410007
        u32 rev;

        // 0x54410008
        struct {
            u16 width, height, depth, line_len;
            u32 base, size;
            u8 red_size, red_pos, green_size, green_pos, blue_size, blue_pos;
            u8 rsvd_size, rsvd_pos;
        } fb;
    };
} packedstruct;

static void parse_atags(ulong base)
{
    struct atag *cur = (struct atag *)base;
    if (!base) {
        return;
    }
    
    lprintf("ARM boot tags: ATAGS @ %lx\n", base);
    
    while (cur && cur->tag) {
        switch (cur->tag) {
        case 0x54410001u:
            lprintf("Core flags: %x, page size: %x, root dev: %x\n",
                cur->core.flags, cur->core.pagesize, cur->core.rootdev
            );
            break;
        case 0x54410002u:
            lprintf("Physical memory start @ %x, size: %x\n",
                cur->mem.start, cur->mem.size
            );
            break;
        case 0x54410009u:
            parse_cmdline(cur->cmdline, (cur->size - 2) * sizeof(u32));
            break;
        default:
            break;
        }
        
        // Move to next tag
        base += cur->size * sizeof(u32);
        cur = (struct atag *)base;
    }
}


/*
 * Boot parameters
 */
#define HAL_AREA_BASE_PADDR     0x100000
#define HAL_AREA_SIZE           0x100000
#define USEABLE_AREA_BASE_PADDR 0x200000


static struct boot_parameters boot_param;

static void build_bootparam()
{
    lprintf("Building boot parameters @ %p ... ", &boot_param);
    
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
    boot_param.hal_vspace_end = HAL_VSPACE_END;
    boot_param.kernel_entry_addr = 0;
    
    // Memory map
    boot_param.hal_page_dir = HAL_AREA_BASE_PADDR + HAL_L1TABLE_OFFSET;
    boot_param.free_addr_start = 2 * 1024 * 1024;
    boot_param.free_pfn_start = ADDR_TO_PFN(boot_param.free_addr_start);
    boot_param.mem_size = (u64)0x40000000ull;
    
    // Memory zones
    u32 zone_count = 0;
    
    // Reserved by HW and loader programs (lowest 64KB)
    boot_param.mem_zones[zone_count].start_paddr = 0x0;
    boot_param.mem_zones[zone_count].len = 0x10000;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // Loader and core image (64KB to 1MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x10000;
    boot_param.mem_zones[zone_count].len = 0x100000 - 0x10000;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // HAL and kernel (1MB to 2MB)
    boot_param.mem_zones[zone_count].start_paddr = HAL_AREA_BASE_PADDR;
    boot_param.mem_zones[zone_count].len = HAL_AREA_SIZE;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // Usable memory (2MB to periph base)
    boot_param.mem_zones[zone_count].start_paddr = USEABLE_AREA_BASE_PADDR;
    boot_param.mem_zones[zone_count].len = BCM2835_BASE - USEABLE_AREA_BASE_PADDR;
    boot_param.mem_zones[zone_count].type = 1;
    zone_count++;
    
    boot_param.mem_zone_count = zone_count;
    
    lprintf("Done!\n");
}

static void build_bootparam_screen()
{
    if (fb_enabled && check_cmdline("screen", "fb")) {
    // Init draw char
//     if (fb_enabled) {
        void *f;
        int w, h, d, p;
        bcm2835_get_framebuffer_info(&f, &w, &h, &d, &p);
        
        boot_param.video_mode = VIDEO_FRAMEBUFFER;
        boot_param.framebuffer_addr = (ulong)f;
        boot_param.res_x = w;
        boot_param.res_y = h;
        boot_param.bytes_per_pixel = d;
        boot_param.bytes_per_line = p;
    } else {
        boot_param.video_mode = VIDEO_UART;
    }
}


/*
 * Paging
 */
#define DRAM_PADDR_START    0x0
#define DRAM_PADDR_END      BCM2835_BASE //0x40000000

static void setup_paging()
{
    int i;
    u32 reg = 0;
    volatile struct l1table *page_table = (volatile struct l1table *)(HAL_AREA_BASE_PADDR + HAL_L1TABLE_OFFSET);
    lprintf("Setup paging @ %p ... ", page_table);
    
    // Set up an 1 to 1 mapping for all 4GB, rw for everyone
    // Note that each entry maps 1MB, and there are 4K entries -> 16KB L1 page table
    for (i = 0; i < 4096; i++) {
        page_table->value_l1section[i].value = 0;
        page_table->value_l1section[i].present = 1;
        page_table->value_l1section[i].user_write = 1;      // AP[1:0] = 01
        page_table->value_l1section[i].user_access = 0;     // Kernel RW, user no access
        page_table->value_l1section[i].sfn = i;
    }
    
    // Enable cache for DRAM
    for (i = DRAM_PADDR_START >> 20; i < (DRAM_PADDR_END >> 20); i++) {
        // TEX[2:0]:C:B = 000:1:1   Cacheable, write-back, write-allocate
        page_table->value_l1section[i].cache_inner = 0x3;
    }
    
    // Map the highest 1MB (HAL + kernel) to physical 1MB to 2MB
    page_table->value_l1section[4095].cache_inner = 0x3;
    page_table->value_l1section[4095].sfn = HAL_AREA_BASE_PADDR >> 20;
    
    // Copy the page table address to cp15
    write_trans_tab_base0(page_table);
//     __asm__ __volatile__ (
//         "mcr p15, 0, %0, c2, c0, 0;"
//         :
//         : "r" (page_table)
//         : "memory"
//     );
    
    // Enable permission check for domain0
    struct domain_access_ctrl_reg domain;
    read_domain_access_ctrl(domain.value);
    domain.domain0 = 0x1;
    write_domain_access_ctrl(domain.value);
    
//     __asm__ __volatile__ (
//         "mcr p15, 0, %0, c3, c0, 0"
//         :
//         : "r" (~0x0)
//     );

    // Enable the MMU
    struct sys_ctrl_reg sys_ctrl;
    read_sys_ctrl(sys_ctrl.value);
    sys_ctrl.mmu_enabled = 1;
    write_sys_ctrl(sys_ctrl.value);
    
//     __asm__ __volatile__ (
//         "mrc p15, 0, %0, c1, c0, 0"
//         : "=r" (reg)
//         :
//         : "cc"
//     );
//     
//     reg |= 0x1;
//     
//     __asm__ __volatile__ (
//         "mcr p15, 0, %0, c1, c0, 0"
//         :
//         : "r" (reg)
//         : "cc"
//     );
    
    // Caches
    lprintf("Dcache: %d\n", sys_ctrl.dcache_enabled);
//     while (1);
    
//     // Check current mode
//     struct proc_status_reg status;
//     read_current_proc_status(status.value);
//     lprintf("Current mode: %x\n", status.mode);
//     
//     panic();
    
    lprintf("Done!\n");
}


/*
 * Core image
 */
extern int __bss_start;
extern int __bss_end;
extern int __end;

static struct coreimg_fat *coreimg = NULL;

static void init_coreimg()
{
    coreimg = (void *)&__end;
    
    boot_param.coreimg_load_addr = (ulong)coreimg;
    
    lprintf("BSS start @ %p, end @ %p, program end @ %p, coreimg @ %p\n",
            &__bss_start, &__bss_end, &__end, coreimg);
}


/*
 * Jump to HAL!
 */
static void (*hal_entry)();

static void jump_to_hal()
{
    
    void (*hal_entry)();
    hal_entry = (void *)boot_param.hal_entry_addr;
    
    lprintf("Starting HAL ... @ %p\n", hal_entry);
    hal_entry(&boot_param);
}


/*
 * Loader entry
 */
void loader_entry(unsigned long r0, unsigned long r1, unsigned long atags)
{
    init_bss();
    init_periph();
    
    lprintf("Welcome to Toddler!\n");
    lprintf("Loader arguments r0: %x, r1: %x, atags: %x\n", r0, r1, atags);
    
    parse_atags(atags);
//     stop(0);
    
    build_bootparam();
    build_bootparam_screen();
    
    setup_paging();
    
    init_coreimg();
    load_images(coreimg, &boot_param, 0, 0);
    
    jump_to_hal();
    
    while (1) {
        lprintf("Should never arrive here!\n");
        panic();
    }
}
