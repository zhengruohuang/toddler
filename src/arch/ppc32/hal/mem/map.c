#include "common/include/data.h"
#include "common/include/memory.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/mem.h"


/*
 * Kernel mapping
 */
static struct page_frame *kernel_pde;
static struct page_frame *kernel_pte;

void kernel_map_per_cpu_area(ulong vaddr, ulong paddr)
{
    int pde_idx = GET_PDE_INDEX(vaddr);
    int pte_idx = GET_PTE_INDEX(vaddr);
    
    assert(kernel_pde->value_pde[pde_idx].next_level);
    
    kernel_pte->value_pte[pte_idx].present = 1;
    kernel_pte->value_pte[pte_idx].pfn = ADDR_TO_PFN(paddr);
}

void init_map()
{
    struct boot_parameters *bp = get_bootparam();
    
    kernel_pde = (struct page_frame *)bp->pde_addr;
    kernel_pte = (struct page_frame *)bp->pte_addr;
}
