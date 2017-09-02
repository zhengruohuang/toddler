#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "common/include/bootparam.h"
#include "common/include/coreimg.h"
#include "common/include/elf32.h"
#include "loader/ofw.h"
#include "loader/mempool.h"


/*
 * Helpers
 */
#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))

static void panic()
{
    ofw_printf("\nPanic!\n");
    
    __asm__ __volatile__
    (
        "xor 3, 3, 3;"
        "addi 3, 3, 0xbe;"
    );
    
    while (1);
}

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
 * Memory layout
 */
static void *claim_virt = NULL;
static void *claim_phys = NULL;
static int claim_size = 0;

static struct pht_group *pht_virt = NULL;
static struct pht_group *pht_phys = NULL;

static struct pht_attri_group *attri_virt = NULL;
static struct pht_attri_group *attri_phys = NULL;

static struct coreimg_fat *coreimg_virt = NULL;
static struct coreimg_fat *coreimg_phys = NULL;

static void *hal_virt = NULL;
static void *hal_phys = NULL;

static struct page_frame *pde_virt = NULL;
static struct page_frame *pde_phys = NULL;

static struct page_frame *pte_virt = NULL;
static struct page_frame *pte_phys = NULL;

static void *mempool_virt = NULL;
static void *mempool_phys = NULL;

static void init_mem_layout(void *coreimg_addr, int coreimg_size)
{
    // Calculate the total size needed
    int pht_area = ALIGN_UP(LOADER_PHT_SIZE, PAGE_SIZE);
    int attri_area = ALIGN_UP(LOADER_PHT_ATTRI_SIZE, PAGE_SIZE);
    int coreimg_area = ALIGN_UP(coreimg_size, PAGE_SIZE);
    int hal_area = ALIGN_UP(HAL_AREA_SIZE, PAGE_SIZE);
    int page_area = PAGE_SIZE + PAGE_SIZE;
    int mempool_area = ALIGN_UP(LOADER_MEMPOOL_SIZE, PAGE_SIZE);
    
    claim_size = pht_area + attri_area + coreimg_area + hal_area + page_area + mempool_area;
    
    // Claim memory
    //   Note here we use PHT size as alignment
    ofw_alloc(&claim_virt, &claim_phys, claim_size, LOADER_PHT_SIZE);
    memzero(claim_virt, claim_size);
    ofw_printf("Memory claimed, virt @ %p, phys @ %p, size: %d\n", claim_virt, claim_phys, claim_size);
    
    // Set up memory areas
    pht_virt = claim_virt;
    pht_phys = claim_phys;
    ofw_printf("PHT area reserved, virt @ %p, phys @ %p, size: %d\n", pht_virt, pht_phys, LOADER_PHT_SIZE);
    
    attri_virt = (void *)((ulong)pht_virt + pht_area);
    attri_phys = (void *)((ulong)pht_phys + pht_area);
    ofw_printf("PHT attribute area reserved, virt @ %p, phys @ %p, size: %d\n", attri_virt, attri_phys, LOADER_PHT_ATTRI_SIZE);
    
    coreimg_virt = (void *)((ulong)attri_virt + attri_area);
    coreimg_phys = (void *)((ulong)attri_phys + attri_area);
    memcpy(coreimg_virt, coreimg_addr, coreimg_size);
    ofw_printf("Core image moved, virt @ %p, phys @ %p, size: %d\n", coreimg_virt, coreimg_phys, coreimg_size);
    
    hal_virt = (void *)((ulong)coreimg_virt + coreimg_area);
    hal_phys = (void *)((ulong)coreimg_phys + coreimg_area);
    ofw_printf("HAL area reserved, virt @ %p, phys @ %p, size: %d\n", hal_virt, hal_phys, hal_area);
    
    pde_virt = (void *)((ulong)hal_virt + hal_area);
    pde_phys = (void *)((ulong)hal_phys + hal_area);
    ofw_printf("PDE area reserved, virt @ %p, phys @ %p, size: %d\n", pde_virt, pde_phys, PAGE_SIZE);
    
    pte_virt = (void *)((ulong)pde_virt + PAGE_SIZE);
    pte_phys = (void *)((ulong)pde_phys + PAGE_SIZE);
    ofw_printf("PTE area reserved, virt @ %p, phys @ %p, size: %d\n", pte_virt, pte_phys, PAGE_SIZE);
    
    mempool_virt = (void *)((ulong)pte_virt + PAGE_SIZE);
    mempool_phys = (void *)((ulong)pte_phys + PAGE_SIZE);
    mempool_init(mempool_virt, mempool_phys, mempool_area);
    ofw_printf("Memory pool created, virt @ %p, phys @ %p, size: %d\n", mempool_virt, mempool_phys, mempool_area);
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

static void detect_screen()
{
    ofw_printf("Detecting screen\n");
    
    screen.graphic = ofw_screen_is_graphic();
    ofw_printf("\tIs graphic: %s\n", screen.graphic ? "yes" : "no");
    
    if (screen.graphic) {
        ofw_fb_info(&screen.fb_addr, &screen.width, &screen.height, &screen.depth, &screen.bpl);
        ofw_printf("\tFramebuffer @ %p -> %p\n\tScreen width: %d, height: %d, depth: %d, bytes per line: %d\n",
            screen.fb_addr, ofw_translate(screen.fb_addr), screen.width, screen.height, screen.depth, screen.bpl
        );
    } else {
        //ofw_escc_info(&screen.serial_addr);
        screen.serial_addr = (void *)0x80013020;
        ofw_printf("\tESCC serial controller @ %p -> %p\n",
            screen.serial_addr, ofw_translate(screen.serial_addr)
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
    
    // AP starter
    boot_param->ap_entry_addr = 0;
    boot_param->ap_page_table_ptr = 0;
    boot_param->ap_stack_top_ptr = 0;
    
    // Video
    if (screen.graphic) {
        boot_param->video_mode = VIDEO_FRAMEBUFFER;
        boot_param->fb_addr = (ulong)screen.fb_addr;
        boot_param->fb_res_x = screen.width;
        boot_param->fb_res_y = screen.height;
        boot_param->fb_bits_per_pixel = screen.depth;
        boot_param->fb_bytes_per_line = screen.bpl;
    } else {
        boot_param->video_mode = VIDEO_SERIAL;
        boot_param->serial_addr = (ulong)screen.serial_addr;
    }
    
    // Core image
    boot_param->coreimg_load_addr = (ulong)coreimg_phys;
    
    // HAL & kernel
    boot_param->hal_start_flag = 0;
    boot_param->hal_entry_addr = 0;
    boot_param->hal_vaddr_end = 0;
    boot_param->hal_vspace_end = HAL_VSPACE_END;
    boot_param->kernel_entry_addr = 0;
    
    // OFW
    boot_param->ofw_tree_addr = (ulong)device_tree;
    
    // Paging
    boot_param->pht_addr = (ulong)pht_phys;
    boot_param->attri_addr = (ulong)attri_phys;
    boot_param->pde_addr = (ulong)pde_phys;
    boot_param->pte_addr = (ulong)pte_phys;
    
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
    ulong left = (hash >> 10) & LOADER_PHT_MASK & 0x1ff;
    ulong right = hash & 0x3ff;
    int index = (int)((left << 10) | right);

    return index;
}

static struct pht_entry *find_free_pht_entry(u32 vaddr, int *group, int *offset)
{
    int i, idx, secondary;
    struct pht_entry *entry = NULL;
    
    for (secondary = 0; secondary <= 1; secondary++) {
        idx = pht_index(vaddr, secondary);
        
        for (i = 0; i < 8; i++) {
            entry = &pht_virt[idx].entries[i];
            if (!entry->valid) {
                if (group) *group = idx;
                if (offset) *offset = i;
                
                entry->secondary = secondary;
                return entry;
            }
        }
    }
    
    ofw_printf("Unable to find a free PHT entry @ %p\n", vaddr);
    return NULL;
}

static void pht_fill(ulong vstart, ulong len, int io)
{
    ulong virt;
    ulong end = vstart + len;
    
    ofw_printf("\tTo fill PHT @ %p to %p\n", vstart, end);
    
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
        
        int group = 0, offset = 0;
        struct pht_entry *entry = find_free_pht_entry(virt, &group, &offset);
        if (entry) {
            // Set up entry
            entry->valid = 1;
            entry->page_idx = (ADDR_TO_PFN(virt) >> 10) & 0x3f;
            entry->vsid = virt >> 28;
            entry->pfn = phys_pfn;
            entry->protect = 0x2;
            if (io) {
                entry->no_cache = 1;
                entry->guarded = 1;
            } else {
                entry->coherent = 1;
            }
            
            // Set up Toddler special attributes
            attri_virt[group].entries[offset].persist = 1;
            
            //ofw_printf("\tPHT filled @ %p: %x %x -> %d\n", virt, entry->word0, entry->word1, phys_pfn);
        } else {
            ofw_printf("\tUnable to find a free PHT entry @ %p\n", virt);
        }
    }
}

static void init_pht()
{
    int i, j;
    int count = LOADER_PHT_SIZE / sizeof(struct pht_group);
    
    ofw_printf("Initializing page hash table\n");
    ofw_printf("\tPHT entries: %d\n", count);
    
    sdr1 = (u32)(ulong)pht_phys | LOADER_PHT_MASK;
    ofw_printf("\tSDR1: %p\n", sdr1);
    
    for (i = 0; i < count; i++) {
        for (j = 0; j < 8; j++) {
            pht_virt[i].entries[j].word0 = 0;
            pht_virt[i].entries[j].word1 = 0;
        }
    }
}

static void init_segment()
{
    int i;
    struct seg_reg sr;
    
    ofw_printf("Initializing segment registers\n");
    
    for (i = 0; i < 16; i++) {
        sr.value = 0;
        sr.key_user = 1;
        sr.vsid = i;
    }
}

static void init_page_table()
{
    int i;
    
    ofw_printf("Initializing page table\n");
    
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
    
    // 4GB-1MB ~ 4GB-4KB -> HAL reserved area
    for (i = 0; i < 255; i++) {
        int idx = 768 + i;
        pte_virt->value_pte[idx].present = 1;
        pte_virt->value_pte[idx].pfn = (u32)ADDR_TO_PFN((ulong)hal_phys) + i;
    }
    
    pht_fill(0xfff00000, 0x100000 - PAGE_SIZE, 0);
    
    // Set up mapping for loader claimed area
    pht_fill((ulong)claim_phys, ALIGN_UP(claim_size, PAGE_SIZE), 0);
    
    // Also set up mapping for framebuffer or serial controller
    if (screen.graphic) {
        ulong fb_size = (ulong)screen.height * (ulong)screen.bpl;
        fb_size = ALIGN_UP(fb_size, PAGE_SIZE);
        pht_fill((ulong)screen.fb_addr, fb_size, 1);
    } else {
        pht_fill((ulong)screen.serial_addr, PAGE_SIZE, 1);
    }
}


/*
 * Load ELF image
 */
static void find_and_layout(char *name, int bin_type)
{
    u32 i;
    
    // Find the file
    u32 file_count = coreimg_virt->header.file_count;
    if (ARCH_BIG_ENDIAN != coreimg_virt->header.big_endian) {
        file_count = swap_endian32(file_count);
    }
    
    struct coreimg_record *record = NULL;
    int found = 0;
    
    for (i = 0; i < file_count; i++) {
        record = &coreimg_virt->records[i];
        
        if (!strdiff(name, (char *)record->file_name, 20)) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        ofw_printf("Unable to find file: %s\n", name);
        panic();
    }
    
    ofw_printf("Loading image: %s\n", name);
    
    // Load the file
    u32 start_offset = record->start_offset;
    if (ARCH_BIG_ENDIAN != coreimg_virt->header.big_endian) {
        start_offset = swap_endian32(start_offset);
    }
    
    struct elf32_header *elf_header = (struct elf32_header *)(start_offset + (ulong)coreimg_virt);
    struct elf32_program *header = NULL;
    ulong vaddr_end = 0;
    
    // Load each segment
    for (i = 0; i < elf_header->elf_phnum; i++) {
        // Move to next header
        if (header) {
            header = (struct elf32_program *)((ulong)header + elf_header->elf_phentsize);
        } else {
            header = (struct elf32_program *)((ulong)elf_header + elf_header->elf_phoff);
        }
        
        ofw_printf("\tSegment @ %p, file: %d, mem: %d", header->program_vaddr,
                   header->program_filesz, header->program_memsz);
        
        // Zero the memory
        if (header->program_memsz) {
            ulong target_vaddr = (ulong)hal_virt + (ulong)header->program_vaddr - HAL_AREA_VADDR;
            memzero((void *)target_vaddr, header->program_memsz);
            
            ofw_printf(", zero section: @ %p (%p)", header->program_vaddr, target_vaddr);
        }
        
        // Copy the program data
        if (header->program_filesz) {
            ulong target_vaddr = (ulong)hal_virt + (ulong)header->program_vaddr - HAL_AREA_VADDR;
            memcpy(
                (void *)target_vaddr,
                (void *)(ulong)(header->program_offset + (u32)elf_header),
                header->program_filesz
            );
            
            ofw_printf(", copy section @ %p -> %p (%p)",
                       header->program_offset + (u32)elf_header,
                       header->program_vaddr, target_vaddr);
        }
        
        ofw_printf("\n");
        
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
        boot_param->hal_entry_addr = elf_header->elf_entry;
        boot_param->hal_vaddr_end = vaddr_end;
        
        ofw_printf("\tHAL entry @ %p, vaddr end @ %p\n", boot_param->hal_entry_addr, boot_param->hal_vaddr_end);
    }
    
    // We just loaded kernel
    else if (bin_type == 2) {
        boot_param->kernel_entry_addr = elf_header->elf_entry;
        ofw_printf("\tKernel entry @ %p\n", boot_param->kernel_entry_addr);
    }
}


/*
 * Real mode
 */
extern void jump_to_real_mode(struct boot_parameters *bp, void *jump_hal_paddr, void *real_entry_paddr);

extern void jump_to_hal(struct boot_parameters *bp, ulong hal_entry_vaddr);
typedef void (*jump_to_hal_t)(struct boot_parameters *bp, ulong hal_entry_vaddr);

static no_opt void real_mode_entry(struct boot_parameters *bp, void *jump_hal_paddr)
{
    int i, j;
    ulong tmp;
    struct seg_reg sr;
    
    // Fill segment registers
    for (i = 0; i < 16; i++) {
        sr.value = 0;
        sr.key_kernel = 1;
        sr.vsid = i;
        
        __asm__ __volatile__
        (
            "mtsrin %[val], %[idx];"
            :
            : [val]"r"(sr.value), [idx]"r"(i << 28)
        );
    }
    
    // Invalidate BAT registers
    __asm__ __volatile__
    (
        "mtspr 528, %[zero]; mtspr 529, %[zero];"
        "mtspr 530, %[zero]; mtspr 531, %[zero];"
        "mtspr 532, %[zero]; mtspr 533, %[zero];"
        "mtspr 534, %[zero]; mtspr 535, %[zero];"
        
        "mtspr 536, %[zero]; mtspr 537, %[zero];"
        "mtspr 538, %[zero]; mtspr 539, %[zero];"
        "mtspr 540, %[zero]; mtspr 541, %[zero];"
        "mtspr 542, %[zero]; mtspr 543, %[zero];"
        :
        : [zero]"r"(0)
    );
    
    // Fill in PHT
    __asm__ __volatile__
    (
        "mtsdr1 %[sdr1];"
        :
        : [sdr1]"r"(bp->pht_addr | LOADER_PHT_MASK)
    );
    
    for (i = 0; i < LOADER_PHT_SIZE / sizeof(struct pht_group); i++) {
        tmp = bp->pht_addr;
        
        for (j = 0; j < 8; j++) {
            __asm__ __volatile__
            (
                "dcbst 0, %[addr];"
                "sync;"
                "icbi 0, %[addr];"
                "sync;"
                "isync;"
                :
                : [addr]"r"(tmp)
            );
            
            tmp += sizeof(struct pht_entry);
        }
    }
    
    // Flush TLB
    __asm__ __volatile__
    (
        "sync;"
    );
    
    tmp = 0;
    for (i = 0; i < 64; i++) {
        __asm__ __volatile__
        (
            "tlbie %[addr];"
            :
            : [addr]"r"(tmp)
        );
        tmp += 0x1000;
    }
    
    __asm__ __volatile__
    (
        "eieio;"
        "tlbsync;"
        "sync;"
    );
    
    // Go to HAL
    jump_to_hal_t jump_to_hal_phys = jump_hal_paddr;
    jump_to_hal_phys(bp, bp->hal_entry_addr);
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
    
    // Memory layout
    init_mem_layout(initrd_addr, initrd_size);
    
    // Various inits
    detect_screen();
    copy_device_tree();
    build_bootparam();
    detect_mem_zones();
    
    // Memory management
    init_segment();
    init_pht();
    init_page_table();
    
    // Load images
    find_and_layout("tdlrhal.bin", 1);
    find_and_layout("tdlrkrnl.bin", 2);
    
    // Go to HAL!
    ofw_printf("Starting HAL @ %p\n\treal_mode_entry @ %p\n\tjump_to_hal @ %p\n",
        boot_param->hal_entry_addr,
        ofw_translate(real_mode_entry), ofw_translate(jump_to_hal)
    );
    jump_to_real_mode(mempool_to_phys(boot_param), ofw_translate(jump_to_hal), ofw_translate(real_mode_entry));
    
    // Should never reach here
    while (1) {
        panic();
    }
}
