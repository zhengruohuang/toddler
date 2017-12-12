#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "common/include/coreimg.h"
#include "common/include/bootparam.h"
#include "loader/include/print.h"
#include "loader/include/lib.h"
#include "loader/include/obp.h"


/*
 * Core image
 */
struct initrd_info {
    char sig[4];
    char pad[12];
    ulong load_addr;
    ulong size;
} packedstruct;

extern int __initrd_info_start;
extern int __initrd_info_end;

static struct coreimg_fat *coreimg;

static void init_coreimg()
{
    struct initrd_info *info = (struct initrd_info *)&__initrd_info_start;
    lprintf("Initrd info start @ %p, end @ %p\n", &__initrd_info_start, &__initrd_info_end);
    
    coreimg = (void *)info->load_addr;
    lprintf("Core image loaded @ %lx, size: %lx\n", info->load_addr, info->size);
}


/*
 * Display
 */
static struct screen_info {
    int graphic;
    
    union {
        struct {
            void *fb_addr;
            int width, height, depth, bpl;
        };
        
        struct {
            void *serial_addr;
        };
    };
} screen;

int screen_is_graphic()
{
    int handle = obp_find_node("options");
    if (!handle) {
        obp_putstr("Unable to find options node!\n");
        panic();
    }
    
    handle = obp_get_prop(handle, "output-device", obp_buf, MAX_OBP_BUF_LEN);
    if (handle <= 0) {
        obp_putstr("Unable to find prop output-device in options node!\n");
        panic();
    }
    
    return !strcmp(obp_buf, "screen");
}

void get_fb_info(void **addr, int *width, int *height, int *depth, int *bpl)
{
    // Open screen device
    int dev = obp_open("screen");
    if (!dev) {
        obp_putstr("Unable to open screen device!\n");
        panic();
    }
    int screen_handle = obp_inst2pkg(dev);
    
    // Get display info
    void *fb_addr;
    int fb_width, fb_height, fb_depth, fb_bpl;
    
    if (obp_get_prop(screen_handle, "_address", &fb_addr, sizeof(fb_addr)) <= 0) {
        fb_addr = NULL;
    }
    if (obp_get_prop(screen_handle, "width", &fb_width, sizeof(fb_width)) <= 0) {
        fb_width = 0;
    }
    if (obp_get_prop(screen_handle, "height", &fb_height, sizeof(fb_height)) <= 0) {
        fb_height = 0;
    }
    if (obp_get_prop(screen_handle, "linebytes", &fb_bpl, sizeof(fb_bpl)) <= 0) {
        fb_bpl = 0;
    }
    
    if (obp_get_prop(screen_handle, "tcx-8-bit", obp_buf, MAX_OBP_BUF_LEN) <= 0) {
        fb_depth = 24;
    } else {
        fb_depth = 8;
    }
    
    // Done
    obp_close(dev);
    
    // Return
    if (addr) *addr = fb_addr;
    if (width) *width = fb_width;
    if (height) *height = fb_height;
    if (depth) *depth = fb_depth;
    if (bpl) *bpl = fb_bpl;
}

void get_serial_info(void **addr)
{
    // Open serial device
    int dev = obp_open("ttya");
    if (!dev) {
        obp_putstr("Unable to open serial device!\n");
        panic();
    }
    int handle = obp_inst2pkg(dev);
    
    // Get serial controller info
    void *serial_addr;
    
    if (obp_get_prop(handle, "address", &serial_addr, sizeof(serial_addr)) <= 0) {
        serial_addr = NULL;
    }
    
    // Done
    obp_close(dev);
    
    // Return
    if (addr) {
        *addr = serial_addr;
    }
}

static void detect_screen()
{
    lprintf("Detecting screen\n");
    
    screen.graphic = screen_is_graphic();
    lprintf("\tIs graphic: %s\n", screen.graphic ? "yes" : "no");
    
    if (screen.graphic) {
        get_fb_info(&screen.fb_addr, &screen.width, &screen.height, &screen.depth, &screen.bpl);
        lprintf("\tFramebuffer @ %p -> %p\n\tScreen width: %d, height: %d, depth: %d, bytes per line: %d\n",
            screen.fb_addr, screen.fb_addr, screen.width, screen.height, screen.depth, screen.bpl
        );
    } else {
        get_serial_info(&screen.serial_addr);
        //screen.serial_addr = (void *)0x80013020;
        lprintf("\tSerial controller @ %p -> %p\n",
            screen.serial_addr, screen.serial_addr
        );
    }
}


/*
 * Device tree
 */
static void duplicate_tree()
{
    void *addr = obp_alloc(1024, 4);
    lprintf("Allocated @ %p\n", addr);
}


/*
 * Boot parameters
 */
static struct boot_parameters boot_param;

static void build_bootparam()
{
    lprintf("Building boot parameters @ %p ... ", &boot_param);
    
    // Boot info
    boot_param.boot_dev = 0;
    boot_param.boot_dev_info = 0;
    
    // AP starter
    boot_param.ap_entry_addr = 0;
    boot_param.ap_page_table_ptr = 0;
    boot_param.ap_stack_top_ptr = 0;
    
//     // Video
//     if (screen.graphic) {
//         boot_param.video_mode = VIDEO_FRAMEBUFFER;
//         boot_param.fb_addr = (ulong)screen.fb_addr;
//         boot_param.fb_res_x = screen.width;
//         boot_param.fb_res_y = screen.height;
//         boot_param.fb_bits_per_pixel = screen.depth;
//         boot_param.fb_bytes_per_line = screen.bpl;
//     } else {
//         boot_param.video_mode = VIDEO_SERIAL;
//         boot_param.serial_addr = (ulong)screen.serial_addr;
//     }
    
    // Core image
    boot_param.coreimg_load_addr = (ulong)coreimg;
    
    // HAL & kernel
    boot_param.hal_start_flag = 0;
    boot_param.hal_entry_addr = 0;
    boot_param.hal_vaddr_end = 0;
    boot_param.hal_vspace_end = HAL_VSPACE_END;
    boot_param.kernel_entry_addr = 0;
    
//     // OFW
//     boot_param.ofw_tree_addr = (ulong)device_tree;
//     
//     // Paging
//     boot_param.pht_addr = (ulong)pht_phys;
//     boot_param.attri_addr = (ulong)attri_phys;
//     boot_param.pde_addr = (ulong)pde_phys;
//     boot_param.pte_addr = (ulong)pte_phys;
    
    // Memory map
    boot_param.free_addr_start = 1 * 1024 * 1024;
    boot_param.free_pfn_start = ADDR_TO_PFN(boot_param.free_addr_start);
    boot_param.mem_size = 0;
    
    // Memory zones
    int zone_count = 0;
    
    // Reserved (lowest 1MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x0;
    boot_param.mem_zones[zone_count].len = 1024 * 1024;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    boot_param.mem_zone_count = zone_count;
    
    lprintf("Done!\n");
}


/*
 * Memory
 */
static void detect_mem()
{
    int idx = 0, err = 0;
    ulong start = 0, size = 0;
    
    idx = 0;
    err = obp_physmem_zone(idx, &start, &size);
    while (!err) {
        lprintf("Physical zone @ %lx, size: %lx\n", start, size);
        
        idx++;
        err = obp_physmem_zone(idx, &start, &size);
    }
    
    idx = 0;
    err = obp_virtmem_zone(idx, &start, &size);
    while (!err) {
        lprintf("Virtual zone @ %lx, size: %lx\n", start, size);
        
        idx++;
        err = obp_virtmem_zone(idx, &start, &size);
    }
}


/*
 * ASI
 */
#define asi_read32(addr, asi, value)            \
    __asm__ __volatile__ (                      \
        "lda [%[regaddr]]" #asi ", %[regval];"  \
        : [regval] "=r" (value)                 \
        : [regaddr] "r" (addr)                  \
    )


/*
 * Paging
 */
struct sparc_ref_mmu {
    union {
        u32 value;
        
        struct {
            u32 impl        : 4;
            u32 ver         : 4;
            u32 sys_ctrl    : 16;
            u32 pso         : 1;
            u32 reserved    : 5;
            u32 no_fault    : 1;
            u32 enabled     : 1;
        };
    } control;
    
    u32 context_table_pointer;
    
    u32 context;
    
    union {
        u32 value;
        
        struct {
            u32 reserved    : 14;
            u32 ext_bus_err : 8;
            u32 level       : 2;
            u32 access_type : 3;
            u32 fault_type  : 3;
            u32 addr_valid  : 1;
            u32 overwrite   : 1;
        };
    } fault_status;
    
    u32 fault_address;
};

static void init_paging()
{
    struct sparc_ref_mmu mmu;
    
    asi_read32(0x000, 0x4, mmu.control.value);
    asi_read32(0x100, 0x4, mmu.context_table_pointer);
    asi_read32(0x200, 0x4, mmu.context);
    asi_read32(0x300, 0x4, mmu.fault_status);
    asi_read32(0x400, 0x4, mmu.fault_address);
    
    lprintf("Enabled: %x, version: %x, partial store order: %x, no fault: %x\n",
        mmu.control.enabled, mmu.control.ver, mmu.control.pso, mmu.control.no_fault
    );
    
    lprintf("Ctrl: %x, ctxt table: %x, ctxt: %x, fault stat: %x, fault addr: %x\n",
        mmu.context_table_pointer, mmu.context, mmu.fault_status.value, mmu.fault_address
    );
}


/*
 * Debug
 */
extern int __bss_start;
extern int __bss_end;

static void show_bss()
{
    int *cur;
    
    lprintf("BSS start @ %p, end @ %p\n", &__bss_start, &__bss_end);
    
    int *start = (int *)ALIGN_UP((ulong)&__bss_start, sizeof(int));
    for (cur = start; cur < &__bss_end; cur++) {
        lprintf("cur @ %p: %d\n", cur, *cur);
    }
}


/*
 * Entry point
 */
void loader_entry(ulong obp_entry)
{
    // BSS
    init_bss();
    
    // OBP
    init_obp(obp_entry);
    lprintf("Toddler loader started!\n");
    show_obp_info();
    
    // Various inits
    init_coreimg();
    detect_screen();
    duplicate_tree();
    build_bootparam();
    detect_mem();
    
    // Memory management
    init_paging();
    
    
//     // Memory layout
//     init_mem_layout(initrd_addr, initrd_size);
//     
//     // Various inits
//     detect_screen();
//     copy_device_tree();
//     build_bootparam();
//     detect_mem_zones();
//     
//     // Memory management
//     init_segment();
//     init_pht();
//     init_page_table();
//     
//     // Load images
//     find_and_layout("tdlrhal.bin", 1);
//     find_and_layout("tdlrkrnl.bin", 2);
//     
//     // Go to HAL!
//     ofw_printf("Starting HAL @ %p\n\treal_mode_entry @ %p\n\tjump_to_hal @ %p\n",
//         boot_param.hal_entry_addr,
//         ofw_translate(real_mode_entry), ofw_translate(jump_to_hal)
//     );
//     jump_to_real_mode(mempool_to_phys(boot_param), ofw_translate(jump_to_hal), ofw_translate(real_mode_entry));
//     
    // Should never reach here
    while (1) {
        //panic();
    }
}
