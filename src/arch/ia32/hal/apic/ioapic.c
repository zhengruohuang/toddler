#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/acpi.h"
#include "hal/include/mps.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/apic.h"


int ioapic_count = 8;


void init_ioapic()
{
    ulong ioapic_paddr = IOAPIC_DEFAULT_PADDR;
    ulong ioapic_vaddr = IOAPIC_TOP_VADDR;
    ioapic_count = 8;
    
    if (acpi_supported && madt_supported) {
        if (madt_ioapic_count < ioapic_count) {
            ioapic_count = madt_ioapic_count;
        }
        assert(ioapic_count > 0);
        
        struct acpi_madt_ioapic *e = NULL;
        int i = 0;
        while (e = get_next_acpi_ioapic_entry(e, &ioapic_paddr)) {
            assert(ioapic_paddr);
            ioapic_vaddr -= PAGE_SIZE;
            
            kernel_indirect_map_array(
                ioapic_vaddr, ioapic_paddr, PAGE_SIZE, 1, 1
            );
            
            kprintf("\tIOAPIC #%d mapped: %p -> %p\n", i, ioapic_vaddr, ioapic_paddr);
            i++;
        }
    }
    
    else if (mps_supported) {
        if (mps_ioapic_count < ioapic_count) {
            ioapic_count = mps_ioapic_count;
        }
        assert(ioapic_count > 0);
        
        struct mps_ioapic *e = NULL;
        int i = 0;
        while (e = get_next_mps_ioapic_entry(e, &ioapic_paddr)) {
            assert(ioapic_paddr);
            ioapic_vaddr -= PAGE_SIZE;
            
            kernel_indirect_map_array(
                IOAPIC_TOP_VADDR - PAGE_SIZE * i, ioapic_paddr,
                PAGE_SIZE, 1, 1
            );
            
            kprintf("\tIOAPIC #%d mapped: %p -> %p\n", i, ioapic_vaddr, ioapic_paddr);
            i++;
        }
    }
    
    // Update HAL virtual space boundary
    get_bootparam()->hal_vspace_end -= PAGE_SIZE * ioapic_count;
}
