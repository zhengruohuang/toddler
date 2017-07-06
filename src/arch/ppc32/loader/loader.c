#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "common/include/memory.h"
#include "loader/ofw.h"
#include "loader/mempool.h"


#define MEMPOOL_SIZE    65536
#define HAL_AREA_SIZE   0x100000    // 1MB
#define PHT_SIZE        65536       // 64KB


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

static struct pht_group *pht_virt = NULL;
static struct pht_group *pht_phys = NULL;

static struct page_frame *pde_virt = NULL;
static struct page_frame *pde_phys = NULL;

static struct page_frame *pte_virt = NULL;
static struct page_frame *pte_phys = NULL;

static void *bsp_area_virt = NULL;
static void *bsp_area_phys = NULL;

static void *mempool_virt = NULL;
static void *mempool_phys = NULL;

static void create_mempool(void *coreimg_addr, int coreimg_size)
{
    // Calculate the total size needed
    int coreimg_area = ALIGN_UP(coreimg_size, PAGE_SIZE);
    int hal_area = ALIGN_UP(HAL_AREA_SIZE, PAGE_SIZE);
    int pht_area = ALIGN_UP(PHT_SIZE + PHT_SIZE, PAGE_SIZE);
    int page_area = PAGE_SIZE + PAGE_SIZE;
    int bsp_area = PAGE_SIZE + PAGE_SIZE;
    int mempool_area = ALIGN_UP(MEMPOOL_SIZE, PAGE_SIZE);
    
    claim_size = coreimg_area + hal_area + pht_area + page_area + bsp_area + mempool_area;
    
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
    
    pht_virt = (struct pht_group *)((ulong)hal_virt + hal_area);
    pht_phys = (struct pht_group *)((ulong)hal_phys + hal_area);
    ofw_printf("PHT area reserved, virt @ %p, phys @ %p, size: %d\n", pht_virt, pht_phys, pht_area);
    
    pde_virt = (struct page_frame *)((ulong)pht_virt + pht_area);
    pde_phys = (struct page_frame *)((ulong)pht_phys + pht_area);
    ofw_printf("PDE area reserved, virt @ %p, phys @ %p, size: %d\n", pde_virt, pde_phys, PAGE_SIZE);
    
    pte_virt = (struct page_frame *)((ulong)pde_virt + PAGE_SIZE);
    pte_phys = (struct page_frame *)((ulong)pde_phys + PAGE_SIZE);
    ofw_printf("PTE area reserved, virt @ %p, phys @ %p, size: %d\n", pte_virt, pte_phys, PAGE_SIZE);
    
    bsp_area_virt = (void *)((ulong)pte_virt + PAGE_SIZE);
    bsp_area_phys = (void *)((ulong)pte_phys + PAGE_SIZE);
    ofw_printf("BSP area reserved, virt @ %p, phys @ %p, size: %d\n", bsp_area_virt, bsp_area_phys, bsp_area);
    
    mempool_virt = (void *)((ulong)bsp_area_virt + bsp_area);
    mempool_phys = (void *)((ulong)bsp_area_phys + bsp_area);
    mempool_init(mempool_virt, mempool_phys, mempool_area);
    ofw_printf("Memory pool created, virt @ %p, phys @ %p, size: %d\n", mempool_virt, mempool_phys, mempool_area);
    
    // Align up PHT address
    pht_virt = (struct pht_group *)ALIGN_UP((ulong)pht_virt, PHT_SIZE);
    pht_phys = (struct pht_group *)ALIGN_UP((ulong)pht_phys, PHT_SIZE);
    ofw_printf("PHT address aligned up, virt @ %p, phys @ %p, size: %d\n", pht_virt, pht_phys, PHT_SIZE);
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
 * Boot parameters
 */
static struct boot_parameters *boot_param;

static void build_bootparam()
{
    boot_param = (struct boot_parameters *)mempool_alloc(sizeof(struct boot_parameters), 0);
    ofw_printf("Building boot parameters @ %x ... ", boot_param);
    
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
    boot_param->coreimg_load_addr = (ulong)coreimg_phys;
    
    // HAL & kernel
    boot_param->hal_start_flag = 0;
    boot_param->hal_entry_addr = 0;
    boot_param->hal_vaddr_end = 0;
    boot_param->hal_vspace_end = 0x80180000;
    boot_param->kernel_entry_addr = 0;
    
    // Memory map
    boot_param->free_addr_start = 1 * 1024 * 1024;
    boot_param->free_pfn_start = ADDR_TO_PFN(boot_param->free_addr_start);
    boot_param->mem_size = 0;
    
    // Memory zones
    int zone_count = 0;
    
    // Reserved (lowest 1MB)
    boot_param->mem_zones[zone_count].start_paddr = 0x0;
    boot_param->mem_zones[zone_count].len = 1024 * 1024;
    boot_param->mem_zones[zone_count].type = 0;
    zone_count++;
    
    boot_param->mem_zone_count = zone_count;
    
    ofw_printf("Done!\n");
}


/*
 * Memory zone
 */
static void detect_mem_zones()
{
    int zone_count = boot_param->mem_zone_count;
    ulong prev_zone_end = 0;
    if (zone_count) {
        prev_zone_end = boot_param->mem_zones[zone_count - 1].start_paddr + boot_param->mem_zones[zone_count - 1].len;
    }
    
    ulong start = 0;
    ulong size = 0;
    int idx = 0;
    
    ofw_printf("Detecting memory zones\n");
    
    int ret = ofw_mem_zone(idx++, &start, &size);
    do {
        ofw_printf("\tStart: %p, size: %x", start, size);
        
        // If there is a hole, mark the hole as unusable
        if (start > prev_zone_end) {
            boot_param->mem_zones[zone_count].start_paddr = prev_zone_end;
            boot_param->mem_zones[zone_count].len = start - prev_zone_end;
            boot_param->mem_zones[zone_count].type = 0;
            
            ofw_printf(", hole @ %p, size: %x",
                (ulong)boot_param->mem_zones[zone_count].start_paddr,
                (ulong)boot_param->mem_zones[zone_count].len
            );
            
            zone_count++;
        }
        
        // If not need to split this zone
        if (start >= prev_zone_end) {
            boot_param->mem_zones[zone_count].start_paddr = start;
            boot_param->mem_zones[zone_count].len = size;
            boot_param->mem_zones[zone_count].type = 1;
            
            ofw_printf(", fully usable zone @ %p, size: %x\n",
                (ulong)boot_param->mem_zones[zone_count].start_paddr,
                (ulong)boot_param->mem_zones[zone_count].len
            );
            
            zone_count++;
            prev_zone_end = start + size;
        }
        
        // Need to split this zone
        else if (start < prev_zone_end && start + size > prev_zone_end) {
            boot_param->mem_zones[zone_count].start_paddr = prev_zone_end;
            boot_param->mem_zones[zone_count].len = start + size - prev_zone_end;
            boot_param->mem_zones[zone_count].type = 1;
            
            ofw_printf(", partially usable zone @ %p, size: %x\n",
                (ulong)boot_param->mem_zones[zone_count].start_paddr,
                (ulong)boot_param->mem_zones[zone_count].len
            );
            
            zone_count++;
            prev_zone_end = start + size;
        }
        
        // Completely overlapped zone
        else {
            ofw_printf(", fully overlapped zone, ignore!\n");
        }
        
        // If the reserved area is inside of current zone, split the current zone
        if ((ulong)claim_phys + claim_size < prev_zone_end) {
            boot_param->mem_zones[zone_count - 1].len =
                (ulong)claim_phys - boot_param->mem_zones[zone_count - 1].start_paddr;
            ofw_printf("\tPrevious zone shrinked @ %p, size: %x\n",
                (ulong)boot_param->mem_zones[zone_count - 1].start_paddr,
                (ulong)boot_param->mem_zones[zone_count - 1].len
            );
            
            boot_param->mem_zones[zone_count].start_paddr = (ulong)claim_phys;
            boot_param->mem_zones[zone_count].len = claim_size;
            boot_param->mem_zones[zone_count].type = 0;
            zone_count++;
            ofw_printf("\tHAL zone inserted @ %p, size: %x\n",
                (ulong)boot_param->mem_zones[zone_count - 1].start_paddr,
                (ulong)boot_param->mem_zones[zone_count - 1].len
            );
            
            boot_param->mem_zones[zone_count].start_paddr = (ulong)claim_phys + claim_size;
            boot_param->mem_zones[zone_count].len = prev_zone_end - ((ulong)claim_phys + claim_size);
            boot_param->mem_zones[zone_count].type = 1;
            zone_count++;
            ofw_printf("\tPrevious zone splitted @ %p, size: %x\n",
                (ulong)boot_param->mem_zones[zone_count - 1].start_paddr,
                (ulong)boot_param->mem_zones[zone_count - 1].len
            );
        }
        
        // Partially overlap
        else if ((ulong)claim_phys < prev_zone_end &&
            (ulong)claim_phys + claim_size >= prev_zone_end
        ) {
            boot_param->mem_zones[zone_count - 1].len =
                (ulong)claim_phys - boot_param->mem_zones[zone_count - 1].start_paddr;
            ofw_printf("\tPrevious zone shrinked @ %p, size: %x\n",
                (ulong)boot_param->mem_zones[zone_count - 1].start_paddr,
                (ulong)boot_param->mem_zones[zone_count - 1].len
            );
            
            boot_param->mem_zones[zone_count].start_paddr = (ulong)claim_phys;
            boot_param->mem_zones[zone_count].len = claim_size;
            boot_param->mem_zones[zone_count].type = 0;
            zone_count++;
            ofw_printf("\tHAL zone inserted @ %p, size: %x\n",
                (ulong)boot_param->mem_zones[zone_count - 1].start_paddr,
                (ulong)boot_param->mem_zones[zone_count - 1].len
            );
        }
        
        // Move to next zone
        ret = ofw_mem_zone(idx++, &start, &size);
    } while (!ret);
    
    // Memory size
    boot_param->mem_zone_count = zone_count;
    boot_param->mem_size = prev_zone_end;
    
    ofw_printf("\tTotal zones: %d, memory size: %x\n", zone_count, prev_zone_end);
}


/*
 * Paging
 */
static u32 sdr1 = 0;

static int pht_index(u32 vaddr, int secondary)
{
    // Calculate the hash
    //   Note for HAL and kernel VSID is just the higher 4 bits of EA
    u32 val_vsid = vaddr >> 28;
    u32 val_pfn = ADDR_TO_PFN(vaddr) & 0xffff;
    u32 hash = val_vsid ^  val_pfn;
    
    // Take care of secondary hash
    if (secondary) {
        hash = ~hash + 1;
    }
    
    // Calculate index
    //   Since we are using the simplist case - 64KB PHT,
    //   the index is simply the lower 10 bits of the hash value
    int index = (int)(hash & 0x3ff);
    return index;
}

static struct pht_entry *find_free_pht_entry(u32 vaddr)
{
    int i, idx;
    struct pht_entry *entry = NULL;
    
    // Primary
    idx = pht_index(vaddr, 0);
    for (i = 0; i < 8; i++) {
        entry = &pht_virt[idx].entries[i];
        if (!entry->valid) {
            return entry;
        }
    }
    
    // Secondary
    idx = pht_index(vaddr, 1);
    for (i = 0; i < 8; i++) {
        entry = &pht_virt[idx].entries[i];
        if (!entry->valid) {
            entry->secondary = 1;
            return entry;
        }
    }
    
    ofw_printf("Unable to find a free PHT entry @ %p\n", vaddr);
    return NULL;
}

static void pht_fill(ulong vstart, ulong len, int io)
{
    ulong virt;
    ulong end = vstart + len;
    
    for (virt = vstart; virt < end; virt += PAGE_SIZE) {
        int pde_idx = GET_PDE_INDEX(virt);
        int pte_idx = GET_PTE_INDEX(virt);
        ulong phys_pfn = 0;
        
        if (pde_virt->value_pde[pde_idx].next_level) {
            phys_pfn = pte_virt->value_pte[pte_idx].pfn;
        } else {
            phys_pfn = pde_virt->value_pde[pde_idx].pfn;
            phys_pfn += ADDR_TO_PFN(virt & 0x3FFFFF);
        }
        
        struct pht_entry *entry = find_free_pht_entry(virt);
        if (entry) {
            entry->valid = 1;
            entry->page_idx = (ADDR_TO_PFN(virt) >> 10) & 0x3f;
            entry->vsid = virt >> 28;
            entry->pfn = phys_pfn;
            entry->protect = 0;
            if (io) {
                entry->no_cache = 1;
                entry->guarded = 1;
            } else {
                entry->coherent = 1;
            }
        }
    }
}

static void init_pht()
{
    int i, j;
    int count = PHT_SIZE / sizeof(struct pht_group);
    
    sdr1 = (u32)(ulong)pht_phys;
    
    for (i = 0; i < count; i++) {
        for (j = 0; j < 8; j++) {
            pht_virt[i].entries[j].word0 = 0;
            pht_virt[i].entries[j].word1 = 0;
        }
    }
}

static void init_paging()
{
    int i;
    
    // Direct mapping for lower 4GB-4MB
    for (i = 0; i < PAGE_ENTRY_COUNT; i++) {
        pde_virt->value_u32[i] = 0;
        pde_virt->value_pde[i].present = 1;
        pde_virt->value_pde[i].supervisor = 1;
        pde_virt->value_pde[i].pfn = (u32)i << PAGE_ENTRY_BITS;
    }
    
    // Init top 4MB mapping
    pde_virt->value_pde[PAGE_ENTRY_COUNT - 1].cache_allow = 1;
    pde_virt->value_pde[PAGE_ENTRY_COUNT - 1].exec_allow = 1;
    pde_virt->value_pde[PAGE_ENTRY_COUNT - 1].write_allow = 1;
    pde_virt->value_pde[PAGE_ENTRY_COUNT - 1].next_level = 1;
    pde_virt->value_pde[PAGE_ENTRY_COUNT - 1].pfn = (u32)ADDR_TO_PFN((ulong)pte_phys);
    
    for (i = 0; i < PAGE_ENTRY_COUNT; i++) {
        pte_virt->value_u32[i] = 0;
        pte_virt->value_pte[i].supervisor = 1;
        pte_virt->value_pte[i].cache_allow = 1;
        pte_virt->value_pte[i].exec_allow = 1;
        pte_virt->value_pte[i].write_allow = 1;
    }
    
    // 4GB-4MB ~ 4GB-4MB+8KB -> BSP reserved area
    for (i = 0; i < 2; i++) {
        pte_virt->value_pte[i].present = 1;
        pte_virt->value_pte[i].exec_allow = 0;
        pte_virt->value_pte[i].pfn = (u32)ADDR_TO_PFN((ulong)bsp_area_phys) + i;
    }
    
    pht_fill(0xffc00000, 8192);
    
    // 4GB-1MB ~ 4GB -> HAL reserved area
    for (i = 0; i < 256; i++) {
        int idx = 768 + i;
        pte_virt->value_pte[idx].present = 1;
        pte_virt->value_pte[idx].pfn = (u32)ADDR_TO_PFN((ulong)hal_phys) + i;
    }
    
    pht_fill(0xfff00000, 0x100000);
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
    detect_screen();
    copy_device_tree();
    build_bootparam();
    detect_mem_zones();
    init_paging();
    
    jump_to_hal();
    
    while (1) {
        //panic();
    }
}
