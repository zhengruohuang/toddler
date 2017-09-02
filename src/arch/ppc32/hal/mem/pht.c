#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "common/include/bootparam.h"
#include "common/include/atomic.h"
#include "hal/include/lib.h"
#include "hal/include/percpu.h"
#include "hal/include/debug.h"
#include "hal/include/int.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"


#define VSID(asid, vaddr)   (((asid << 4) | (vaddr >> 28)) & 0x7ffff)
#define API(vaddr)          ((ADDR_TO_PFN(vaddr) >> 10) & 0x3f)

dec_per_cpu(ulong, cur_page_dir_pfn);
dec_per_cpu(struct page_frame *, cur_page_dir);

static struct pht_group *loader_pht;

static struct pht_group *pht;
static struct pht_attri_group *attri;

static ulong pht_size;
static ulong pht_mask;


/*
 * Spin lock
 */
typedef struct {
    union {
        volatile ulong value;
        struct {
            volatile ulong locked       : 1;
            volatile ulong int_enabled  : 1;
        };
    };
} spinlock_t;

static spinlock_t pht_lock;

static void spin_init(spinlock_t *lock)
{
    lock->value = 0;
}

static void spin_lock_int(spinlock_t *lock)
{
    if (!work_started) {
        return;
    }
    
    int enabled = disable_local_int();
    
    do {
        do {
        } while (lock->value);
    } while (!atomic_cas(&lock->value, 0, 1));
    
    spinlock_t newlock;
    newlock.locked = 1;
    newlock.int_enabled = enabled;
    
    assert(atomic_cas(&lock->value, 1, newlock.value));
}

static void spin_unlock_int(spinlock_t *lock)
{
    if (!work_started) {
        return;
    }
    
    int enabled = lock->int_enabled;
    
    assert(lock->locked);
    lock->value = 0;
    
    restore_local_int(enabled);
}


/*
 * General PHT operations
 */
static int pht_index(ulong asid, ulong vaddr, int secondary)
{
    // Calculate the hash
    ulong val_vsid = VSID(asid, vaddr);
    ulong val_pfn = ADDR_TO_PFN(vaddr) & 0xffff;
    ulong hash = val_vsid ^ val_pfn;
    
    // Take care of secondary hash
    if (secondary) {
        hash = ~hash + 1;
    }
    
    // Calculate index
    ulong left = (hash >> 10) & pht_mask & 0x1ff;
    ulong right = hash & 0x3ff;
    int index = (int)((left << 10) | right);
    
    return index;
}

static struct pht_entry *find_pht_entry(ulong asid, ulong vaddr, int *group, int *offset)
{
    int i, idx, secondary;
    struct pht_entry *entry = NULL;
    
    u32 page_idx = API(vaddr);
    u32 vsid = VSID(asid, vaddr);
    
    for (secondary = 0; secondary <= 1; secondary++) {
        idx = pht_index(asid, vaddr, secondary);
        
        for (i = 0; i < 8; i++) {
            entry = &pht[idx].entries[i];
            if (
                entry->valid &&
                (entry->page_idx == page_idx) &&
                (entry->vsid == vsid)
            ) {
                if (group) *group = idx;
                if (offset) *offset = i;
                
                return entry;
            }
        }
    }
    
    return NULL;
}

static struct pht_entry *find_free_pht_entry(ulong asid, ulong vaddr, int *group, int *offset)
{
    int i, idx, secondary;
    struct pht_entry *entry = NULL;
    
    for (secondary = 0; secondary <= 1; secondary++) {
        idx = pht_index(asid, vaddr, secondary);
        
        for (i = 0; i < 8; i++) {
            entry = &pht[idx].entries[i];
            if (!entry->valid) {
                if (group) *group = idx;
                if (offset) *offset = i;
                
                entry->secondary = secondary;
                return entry;
            }
        }
    }
    
    kprintf("\t\tUnable to find a free PHT entry @ %p\n", (void *)vaddr);
    return NULL;
}

static struct pht_entry *evict_pht_entry(ulong asid, ulong vaddr, int *group, int *offset)
{
    /*
        lock(PTE)
        PTE[V] ← 0      // (other fields don’t matter)
        sync            // ensure update completed
        tlbie(old_EA)   // invalidate old translation
        eieio           // order tlbie before tlbsync
        tlbsync         // ensure tlbie completed on all processors
        sync            // ensure tlbsync completed
        unlock(PTE)
     */
    
    int i, idx, secondary;
    struct pht_entry *entry = NULL;
    struct pht_attri_entry *attri_entry = NULL;
    
    u32 page_idx = API(vaddr);
    u32 vsid = VSID(asid, vaddr);
    
    for (secondary = 0; secondary <= 1; secondary++) {
        idx = pht_index(asid, vaddr, secondary);
        
        for (i = 0; i < 8; i++) {
            entry = &pht[idx].entries[i];
            attri_entry = &attri[idx].entries[i];
            
            if (
                !attri_entry->persist &&
                entry->valid &&
                (entry->page_idx == page_idx) &&
                (entry->vsid == vsid)
            ) {
                if (group) *group = idx;
                if (offset) *offset = i;
                
                entry->word0 = entry->word1 = 0;
                entry->secondary = secondary;
                attri_entry->value = 0;
                
                __asm__ __volatile__
                (
                    "sync;"
                    "tlbie %[vaddr];"
                    "eieio;"
                    "tlbsync;"
                    "sync;"
                    :
                    : [vaddr]"r"(vaddr)
                    : "cc"
                );
                
                //warn("PHT entry evicted @ %p\n", (void *)vaddr);
                return entry;
            }
        }
    }
    
    kprintf("\t\tUnable to evict a PHT entry @ %p\n", (void *)vaddr);
    return NULL;
}

void fill_pht_by_page(ulong asid, ulong vaddr, ulong ppfn, ulong count, int io, int priority)
{
    /*
        lock(PTE)
        PTE[RPN,R,C,WIMG,PP] ← new values
        eieio //order 1st PTE update before 2nd
        PTE[VSID,H,API,V] ← new values (V = 1)
        sync // ensure updates completed
        unlock(PTE)
     */
    
    ulong i;
    ulong virt = ALIGN_DOWN(vaddr, PAGE_SIZE);
    ulong phys_pfn = ppfn;
    
    // FIXME: Need to lock PHT first
    spin_lock_int(&pht_lock);
    
    for (i = 0; i < count; i++) {
        // First see if the entry already exists
        int group = 0, offset = 0;
        struct pht_entry *entry = find_pht_entry(asid, virt, &group, &offset);
        
        // The entry does not exist, create a new one
        if (!entry) {
            entry = find_free_pht_entry(asid, virt, &group, &offset);
            //kprintf("\tCreate new PHT entry @ %p\n", entry);
            
            // An eviction is required, this has to be successful
            if (!entry) {
                entry = evict_pht_entry(asid, virt, &group, &offset);
                //kprintf("\tEvict new PHT entry @ %p\n", entry);
            }
        }
        
        // Make sure we found a valid free PHT entry
        assert(entry);
        
        // Set priority
        switch (priority) {
        case KERNEL_PHT_PERSIST:
            attri[group].entries[offset].temporary = 0;
            attri[group].entries[offset].persist = 1;
            break;
        case KERNEL_PHT_REGULAR:
            attri[group].entries[offset].temporary = 0;
            break;
        case KERNEL_PHT_TEMPORARY:
            if (!attri[group].entries[offset].persist) {
                attri[group].entries[offset].temporary = 1;
            }
            break;
        default:
            break;
        }
        
        // Set up the entry
        struct pht_entry val;
        val.word0 = val.word1 = 0;
        val.secondary = entry->secondary;
        
        val.valid = 1;
        val.page_idx = API(virt);
        val.vsid = VSID(asid, virt);
        val.pfn = phys_pfn;
        val.protect = 0x2;
        if (io) {
            val.no_cache = 1;
            val.guarded = 1;
        } else {
            val.coherent = 1;
        }
        
        __asm__ __volatile__
        (
            "stw %[word1], 0(%[ptr1]);"
            "eieio;"
            "stw %[word0], 0(%[ptr0]);"
            "sync;"
            :
            : [word0]"r"(val.word0), [ptr0]"r"((ulong)entry),
              [word1]"r"(val.word1), [ptr1]"r"((ulong)entry + sizeof(ulong))
            : "cc"
        );
        
//         entry->valid = 1;
//         entry->page_idx = API(virt);
//         entry->vsid = VSID(asid, virt);
//         entry->pfn = phys_pfn;
//         entry->protect = 0x2;
//         if (io) {
//             entry->no_cache = 1;
//             entry->guarded = 1;
//         } else {
//             entry->coherent = 1;
//         }
    
        // Move to next page
        virt += PAGE_SIZE;
        phys_pfn++;
    }
    
    spin_unlock_int(&pht_lock);
}

void fill_pht_by_addr(ulong asid, ulong vaddr, ulong paddr, ulong size, int io, int priority)
{
    ulong ppfn = ADDR_TO_PFN(paddr);
    ulong vstart = ALIGN_DOWN(vaddr, PAGE_SIZE);
    ulong vend = ALIGN_UP(vaddr + size, PAGE_SIZE);
    ulong pages = (vend - vstart) >> PAGE_BITS;
    
//     if (asid) {
//         kprintf("To fill vstart @ %p, vend @ %p, ppfn @ %p, pages: %d\n",
//             (void *)vstart, (void *)vend, (void *)ppfn, (int)pages);
//     }
    fill_pht_by_page(asid, vstart, ppfn, pages, io, priority);
}

ulong translate_pht(ulong asid, ulong vaddr)
{
    // FIXME: lock PHT
    spin_lock_int(&pht_lock);
    
    ulong paddr = 0;
    struct pht_entry *entry = find_pht_entry(asid, vaddr, NULL, NULL);
    
    if (entry) {
        paddr = PFN_TO_ADDR((ulong)entry->pfn);
        paddr |= vaddr & (PAGE_SIZE - 1);
    }
    
    spin_unlock_int(&pht_lock);
    
    return paddr;
}

int evict_pht(ulong asid, ulong vaddr)
{
    // FIXME: lock PHT
    spin_lock_int(&pht_lock);
    
    int evicted = 0;
    struct pht_entry *entry = evict_pht_entry(asid, vaddr, NULL, NULL);
    
    if (entry) {
        evicted = 1;
    }
    
    spin_unlock_int(&pht_lock);
    
    return evicted;
}


/*
 * Kernel PHT
 */
static struct page_frame *kernel_pde;
static struct page_frame *kernel_pte;

void fill_kernel_pht(ulong vstart, ulong len, int io, int priority)
{
    ulong end = ALIGN_UP(vstart + len, PAGE_SIZE);
    ulong virt = ALIGN_DOWN(vstart, PAGE_SIZE);
    
    kprintf("\tTo fill kernel PHT @ %p to %p\n", (void *)vstart, (void *)end);
    
    for (; virt < end; virt += PAGE_SIZE) {
        int pde_idx = GET_PDE_INDEX(virt);
        int pte_idx = GET_PTE_INDEX(virt);
        ulong phys_pfn = 0;
        
        if (kernel_pde->value_pde[pde_idx].next_level) {
            phys_pfn = kernel_pte->value_pte[pte_idx].pfn;
        } else {
            phys_pfn = kernel_pte->value_pde[pde_idx].pfn;
            phys_pfn += ADDR_TO_PFN(virt & 0x3FFFFF);
        }
        
        kprintf("\t\tPage @ %p -> %p\n", (void *)virt, (void *)ADDR_TO_PFN(phys_pfn));
        fill_pht_by_page(0, virt, phys_pfn, 1, io, priority);
    }
}


/*
 * Loader PHT
 */
static int loader_pht_index(u32 vaddr, int secondary)
{
    // Calculate the hash
    //   Note for HAL and kernel VSID is just the higher 4 bits of EA
    u32 val_vsid = vaddr >> 28;
    u32 val_pfn = ADDR_TO_PFN(vaddr) & 0xffff;
    u32 hash = val_vsid ^ val_pfn;
    
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

static struct pht_entry *find_loader_pht_entry(u32 vaddr, int *group, int *offset)
{
    int i, idx, secondary;
    struct pht_entry *entry = NULL;
    
    u32 page_idx = (ADDR_TO_PFN(vaddr) >> 10) & 0x3f;
    u32 vsid = vaddr >> 28;
    
    for (secondary = 0; secondary <= 1; secondary++) {
        idx = loader_pht_index(vaddr, secondary);
        
        for (i = 0; i < 8; i++) {
            entry = &loader_pht[idx].entries[i];
            if (
                entry->valid &&
                (entry->page_idx == page_idx) &&
                (entry->vsid == vsid)
            ) {
                if (group) *group = idx;
                if (offset) *offset = i;
                
                return entry;
            }
        }
    }
    
    return NULL;
}

static struct pht_entry *find_free_loader_pht_entry(u32 vaddr, int *group, int *offset)
{
    int i, idx, secondary;
    struct pht_entry *entry = NULL;
    
    for (secondary = 0; secondary <= 1; secondary++) {
        idx = loader_pht_index(vaddr, secondary);
        
        for (i = 0; i < 8; i++) {
            entry = &loader_pht[idx].entries[i];
            if (!entry->valid) {
                if (group) *group = idx;
                if (offset) *offset = i;
                
                entry->secondary = secondary;
                return entry;
            }
        }
    }
    
    kprintf("Unable to find a free loader PHT entry @ %p\n", (void *)(ulong)vaddr);
    return NULL;
}


static void fill_loader_pht(ulong vstart, ulong len, int io)
{
    ulong virt;
    ulong end = vstart + len;
    
    kprintf("\tTo fill loader PHT @ %p to %p\n", (void *)vstart, (void *)end);
    
    for (virt = vstart; virt < end; virt += PAGE_SIZE) {
        int pde_idx = GET_PDE_INDEX(virt);
        int pte_idx = GET_PTE_INDEX(virt);
        ulong phys_pfn = 0;
        
        if (kernel_pde->value_pde[pde_idx].next_level) {
            phys_pfn = kernel_pte->value_pte[pte_idx].pfn;
        } else {
            phys_pfn = kernel_pte->value_pde[pde_idx].pfn;
            phys_pfn += ADDR_TO_PFN(virt & 0x3FFFFF);
        }
        
        int group = 0, offset = 0;
        struct pht_entry *entry = find_free_loader_pht_entry(virt, &group, &offset);
        assert(entry);
        
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
        
        //kprintf("\t\tPHT filled @ %p: %x %x -> %p\n", (void *)virt, entry->word0, entry->word1, (void *)phys_pfn);
    }
}


/*
 * Initialization
 */
static void apply_pht()
{
    ulong sdr1 = (ulong)pht | pht_mask;
    
    kprintf("\tTo switch to new PHT @ %p\n", (void *)sdr1);
    
    __asm__ __volatile__
    (
        "sync;"
        "mtsdr1 %[val];"
        "sync;"
        "isync;"
        :
        : [val]"r"(sdr1)
    );
    
    //while (1);
}

static void copy_loader_pht(ulong mem_size)
{
    ulong vaddr = 0, ppfn = 0;
    
    for (; vaddr < 0xFFFFF000; vaddr += PAGE_SIZE, ppfn++) {
        // First see if this entry exists in loader PHT
        int group = 0, offset = 0;
        struct pht_entry *entry = find_loader_pht_entry(vaddr, &group, &offset);
        
        // Fill the new PHT
        if (entry) {
            fill_pht_by_page(0, vaddr, entry->pfn, 1, entry->guarded, KERNEL_PHT_PERSIST);
            //kprintf("\t\tLoader map detected @ %p -> %p\n", (void *)vaddr, (void *)(ulong)entry->pfn);
        } else if (vaddr < mem_size) {
            fill_pht_by_page(0, vaddr, ppfn, 1, 0, KERNEL_PHT_REGULAR);
        }
    }
}

static void alloc_pht()
{
    int i;
    struct boot_parameters *bp = get_bootparam();
    
    int zone_idx = -1;
    ulong zone_start = -1;
    ulong zone_len = 0;
    
    // Find out PHT size, simply use fixed 4MB for now
    pht_size = 0x400000;
    pht_mask = 0x3f;    // 0011 1111
    int pht_group_count = pht_size / sizeof(struct pht_group);
    ulong attri_size = sizeof(struct pht_attri_group) * pht_group_count;
    
    // Go through memory zones and find the biggest zone
    int zone_count = bp->mem_zone_count;
    for (i = 0; i < zone_count; i++) {
        struct boot_mem_zone *cur_zone = &bp->mem_zones[i];
        int type = cur_zone->type;
        ulong start = (ulong)cur_zone->start_paddr;
        ulong len = (ulong)cur_zone->len;
        
        if (type && len > zone_len) {
            zone_idx = i;
            zone_start = start;
            zone_len = len;
        }
    }
    
    // Find out PHT address
    ulong alloc_start = zone_start + zone_len - (pht_size + attri_size);
    alloc_start = ALIGN_DOWN(alloc_start, pht_size);
    pht = (void *)alloc_start;
    attri = (void *)(alloc_start + pht_size);
    
    // Split the zone
    for (i = zone_count - 1; i > zone_idx; i--) {
        bp->mem_zones[i + 1] = bp->mem_zones[i];
    }
    bp->mem_zone_count++;
    
    ulong old_len = (ulong)bp->mem_zones[zone_idx].len;
    bp->mem_zones[zone_idx].len = alloc_start - (ulong)bp->mem_zones[zone_idx].start_paddr;
    
    bp->mem_zones[zone_idx + 1].start_paddr = alloc_start;
    bp->mem_zones[zone_idx + 1].len = old_len - (ulong)bp->mem_zones[zone_idx].len;
    bp->mem_zones[zone_idx + 1].type = 0;
    
    // Fill loader PHT for the newly allocated area
    fill_loader_pht(alloc_start, pht_size + attri_size, 0);
}

void init_pht()
{
    struct boot_parameters *bp = get_bootparam();
    
    kprintf("Initializing PHT\n");
    
    // Init PHT lock
    spin_init(&pht_lock);
    
    // Obtain kernel page table
    kernel_pde = (struct page_frame *)bp->pde_addr;
    kernel_pte = (struct page_frame *)bp->pte_addr;
    
    // Set up loader PHT
    pht = (void *)bp->pht_addr;
    attri = (void *)bp->attri_addr;
    
    pht_size = LOADER_PHT_SIZE;
    pht_mask = LOADER_PHT_MASK;
    
    kprintf("\tLoader PHT @ %p, attributes @ %p, size: %p, mask: %p\n", pht, attri, (void *)pht_size, (void *)pht_mask);
    
    // Set up kernel full map
    fill_pht_by_addr(0, 0, 0, (ulong)bp->mem_size, 0, KERNEL_PHT_REGULAR);
    kprintf("\tFull mapping created for kernel\n");
    
//     // First set up loader PHT
//     loader_pht = (void *)bp->pht_addr;
//     kprintf("\tLoader PHT @ %p\n", loader_pht);
//     
//     // Allocate the new PHT
//     alloc_pht();
//     kprintf("\tPHT @ %p, attributes @ %p\n", pht, attri);
//     
//     // Init the new PHT
//     copy_loader_pht((ulong)bp->mem_size);
//     
//     // Make the new PHT effective
//     apply_pht();
}
