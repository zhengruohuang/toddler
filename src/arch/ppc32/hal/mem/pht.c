#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/print.h"


static struct pht_group *pht;
static struct pht_attri_group *attri;


/*
 * Kernel PHT
 */
static struct page_frame *kernel_pde;
static struct page_frame *kernel_pte;

static int kernel_pht_index(u32 vaddr, int secondary)
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

static struct pht_entry *find_free_kernel_pht_entry(u32 vaddr, int *group, int *offset)
{
    int i, idx;
    struct pht_entry *entry = NULL;
    
    // Primary
    idx = kernel_pht_index(vaddr, 0);
    for (i = 0; i < 8; i++) {
        entry = &pht[idx].entries[i];
        if (!entry->valid) {
            if (group) *group = idx;
            if (offset) *offset = i;
            
            entry->secondary = 0;
            return entry;
        }
    }
    
    // Secondary
    idx = kernel_pht_index(vaddr, 1);
    for (i = 0; i < 8; i++) {
        entry = &pht[idx].entries[i];
        if (!entry->valid) {
            if (group) *group = idx;
            if (offset) *offset = i;
            
            entry->secondary = 1;
            return entry;
        }
    }
    
    kprintf("Unable to find a free kernel PHT entry @ %p\n", (void *)vaddr);
    return NULL;
}


void fill_kernel_pht(ulong vstart, ulong len, int io, int persist)
{
    ulong virt;
    ulong end = vstart + len;
    
    kprintf("\tTo fill kernel PHT @ %p to %p\n", (void *)vstart, (void *)end);
    
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
        struct pht_entry *entry = find_free_kernel_pht_entry(virt, &group, &offset);
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
            attri[group].entries[offset].persist = persist;
            
            kprintf("\t\tPHT filled @ %p: %x %x -> %p\n", (void *)virt, entry->word0, entry->word1, (void *)phys_pfn);
        } else {
            kprintf("\t\tUnable to find a free PHT entry @ %p\n", (void *)virt);
        }
    }
}

void evict_kernel_pht(ulong vstart, ulong len)
{
    int i, group, second;
    struct pht_entry *entry = NULL;
    
    ulong virt;
    ulong end = vstart + len;
    
    kprintf("\tTo evict kernel PHT @ %p to %p\n", (void *)vstart, (void *)end);
    
    for (virt = vstart; virt < end; virt += PAGE_SIZE) {
        for (second = 0; second < 2; second++) {
            group = kernel_pht_index(virt, second);
            
            for (i = 0; i < 8; i++) {
                entry = &pht[group].entries[i];
                
                if (
                    (entry->valid) &&
                    (entry->page_idx == ((ADDR_TO_PFN(virt) >> 10) & 0x3f)) &&
                    (entry->vsid == virt >> 28)
                ) {
                    entry->word0 = entry->word1 = 0;
                    attri[group].entries[i].value = 0;
                    
                    kprintf("\t\tPHT evicted @ %p\n", (void *)virt);
                }
            }
        }
    }
}

void init_pht()
{
    struct boot_parameters *bp = get_bootparam();
    
    kprintf("Initializing PHT\n");
    
    pht = (void *)bp->pht_addr;
    attri = (void *)bp->attri_addr;
    
    kernel_pde = (struct page_frame *)bp->pde_addr;
    kernel_pte = (struct page_frame *)bp->pte_addr;
    
    kprintf("\tPHT @ %p, attributes @ %p\n", pht, attri);
}
