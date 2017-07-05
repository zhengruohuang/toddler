#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "common/include/memory.h"
#include "loader/ofw.h"
#include "loader/mempool.h"


#define MEMPOOL_SIZE    65536


/*
 * Helper functions
 */
static void memzero(void *src, int size)
{
    int i;
    char *ptr = (char *)src;
    
    for (i = 0; i < size; i++) {
        *(ptr++) = 0;
    }
    
}

static void memcpy(void *dest, const void *src, int count)
{
    int i;
    
    char *s = (char *)src;
    char *d = (char *)dest;
    
    for (i = 0; i < count; i++) {
        *(d++) = *(s++);
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
 * Memory pool
 */
static void *claim_virt = NULL;
static void *claim_phys = NULL;
static int claim_size = 0;

static void *coreimg_virt = NULL;
static void *coreimg_phys = NULL;

static void *hal_virt = NULL;
static void *hal_phys = NULL;

static void *mempool_virt = NULL;
static void *mempool_phys = NULL;

static void create_mempool(void *coreimg_addr, int coreimg_size)
{
    // Calculate the total size needed
    int coreimg_area = ALIGN_UP(coreimg_size, PAGE_SIZE);
    int hal_area = ALIGN_UP(0x100000, PAGE_SIZE);
    int mempool_area = ALIGN_UP(MEMPOOL_SIZE, PAGE_SIZE);
    
    claim_size = coreimg_area + mempool_area + hal_area;
    
    // Claim memory
    ofw_alloc(&claim_virt, &claim_phys, claim_size);
    memzero(claim_virt, claim_size);
    ofw_printf("Memory claimed, virt @ %p, phys @ %p, size: %d\n", claim_virt, claim_phys, claim_size);
    
    // Set up memory areas
    coreimg_virt = claim_virt;
    coreimg_phys = claim_phys;
    memcpy(coreimg_virt, coreimg_addr, coreimg_size);
    ofw_printf("Core image moved, virt @ %p, phys @ %p, size: %d\n", coreimg_virt, coreimg_phys, coreimg_size);
    
    hal_virt = (void *)((ulong)coreimg_virt + coreimg_area);
    hal_phys = (void *)((ulong)coreimg_phys + coreimg_area);
    ofw_printf("HAL area reserved, virt @ %p, phys @ %p, size: %d\n", hal_virt, hal_phys, hal_area);
    
    mempool_virt = (void *)((ulong)hal_virt + hal_area);
    mempool_phys = (void *)((ulong)hal_phys + hal_area);
    mempool_init(mempool_virt, mempool_phys, mempool_area);
    ofw_printf("Memory pool created, virt @ %p, phys @ %p, size: %d\n", mempool_virt, mempool_phys, mempool_area);
}


/*
 * Memory zone
 */
static void detect_mem_zones()
{
    ulong start = 0;
    ulong size = 0;
    int idx = 0;
    
    ofw_printf("Detecting memory zones\n");
    
    int ret = ofw_mem_zone(idx++, &start, &size);
    do {
        ofw_printf("\tStart: %p, size: %x\n", start, size);
        ret = ofw_mem_zone(idx++, &start, &size);
    } while (!ret);
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
            void *uart_addr;
        };
    };
} screen;

static void detect_screen()
{
    ofw_printf("Detecting screen\n");
    
    screen.graphic = ofw_screen_is_graphic();
    ofw_printf("\tIs graphic: %s\n", screen.graphic ? "yes" : "no");
    
    if (screen.graphic) {
        ofw_screen_info(&screen.fb_addr, &screen.width, &screen.height, &screen.depth, &screen.bpl);
        ofw_printf("\tFramebuffer @ %p -> %p\n\tScreen width: %d, height: %d, depth: %d, bytes per line: %d\n",
            screen.fb_addr, ofw_translate(screen.fb_addr), screen.width, screen.height, screen.depth, screen.bpl
        );
        
    }
}


/*
 * Device tree
 */
static struct ofw_tree_node *device_tree;

static void copy_device_tree()
{
    device_tree = ofw_tree_build();
}


/*
 * MMU and caches
 */
static void init_mmu()
{
}

static void init_caches()
{
}


/*
 * Boot parameters
 */
static struct boot_parameters *boot_param;

static void build_bootparam()
{
    boot_param = (struct boot_parameters *)mempool_alloc(sizeof(struct boot_parameters), 0);
    ofw_printf("Building boot parameters @ %x", boot_param);
    
    // Boot info
    boot_param->boot_dev = 0;
    boot_param->boot_dev_info = 0;
    
    // Loader func
    boot_param->loader_func_type_ptr = 0;
    
    // AP starter
    boot_param->ap_entry_addr = 0;
    boot_param->ap_page_table_ptr = 0;
    boot_param->ap_stack_top_ptr = 0;
    
    // Core image
    //boot_param->coreimg_load_addr = coreimg_start_addr;
    
    // HAL & kernel
    boot_param->hal_start_flag = 0;
    boot_param->hal_entry_addr = 0;
    boot_param->hal_vaddr_end = 0;
    boot_param->hal_vspace_end = 0x80180000;
    boot_param->kernel_entry_addr = 0;
    
    // Memory map
    boot_param->free_addr_start = 2 * 1024 * 1024;
    //boot_param->free_pfn_start = ADDR_TO_PFN(boot_param->free_addr_start);
    //boot_param->mem_size = (u64)memory_size;
    
    // Memory zones
    u32 zone_count = 0;
    
    // Reserved by HW and loader programs (lowest 64KB)
    boot_param->mem_zones[zone_count].start_paddr = 0x0;
    boot_param->mem_zones[zone_count].len = 0x8000;
    boot_param->mem_zones[zone_count].type = 0;
    zone_count++;
    
    // Core image (64KB to 1MB)
    boot_param->mem_zones[zone_count].start_paddr = 0x8000;
    boot_param->mem_zones[zone_count].len = 0x100000 - 0x8000;
    boot_param->mem_zones[zone_count].type = 0;
    zone_count++;
    
    // HAL and kernel (1MB to 2MB)
    boot_param->mem_zones[zone_count].start_paddr = 0x100000;
    boot_param->mem_zones[zone_count].len = 0x100000;
    boot_param->mem_zones[zone_count].type = 0;
    zone_count++;
    
    // Usable memory (2MB to 128MB)
    boot_param->mem_zones[zone_count].start_paddr = 0x200000;
    //boot_param->mem_zones[zone_count].len = memory_size - 0x200000;
    boot_param->mem_zones[zone_count].type = 1;
    zone_count++;
    
    boot_param->mem_zone_count = zone_count;
    
    //lprintf("Done!\n");
}


/*
 * Jump to HAL!
 */
static void (*hal_entry)();

static void jump_to_hal()
{

}


/*
 * Entry point
 */
void loader_entry(void *initrd_addr, ulong initrd_size, ulong ofw_entry)
{
    init_bss();
    ofw_init(ofw_entry);
    ofw_printf("Toddler loader started!\n");
    ofw_printf("Boot args: initrd @ %p, size: %d, OFW @ %p\n", initrd_addr, initrd_size, ofw_entry);
    
    create_mempool(initrd_addr, initrd_size);
    detect_mem_zones();
    detect_screen();
    copy_device_tree();
    
    build_bootparam();
    
    init_mmu();
    init_caches();
    
    jump_to_hal();
    
    while (1) {
        //panic();
    }
}
