// #include "common/include/data.h"
// #include "common/include/memlayout.h"
// #include "common/include/memory.h"
// #include "hal/include/print.h"
// #include "hal/include/lib.h"
// #include "hal/include/acpi.h"
// #include "hal/include/mps.h"
// #include "hal/include/mem.h"
// #include "hal/include/cpu.h"
// #include "hal/include/apic.h"
// #include "hal/include/i8259a.h"
// 
// 
// int apic_supported = 0;
// int apic_present = 0;
// int bsp_lapic_id = 0;
// int ioapic_count = 0;
// 
// 
// static ulong cpu_apic_present()
// {
//     struct cpuid_reg reg;
//     
//     reg.a = 1;
//     reg.b = 0;
//     reg.c = 0;
//     reg.d = 0;
//     cpuid(&reg);
//     
//     return (reg.d & (0x1 << 9));
// }
// 
// void start_apic()
// {
//     if (apic_supported) {
//         start_ioapic();
//     } else {
//         start_i8259a();
//     }
// }
// 
// static void map_apic_mp()
// {
//     u32 lapic_address_physical = LAPIC_DEFAULT_PHYSICAL_ADDR;
//     
//     /* Get the physical address of LAPIC and IOAPIC from ACPI or MP Specification */
//     if (acpi_supported && madt_supported) {
//         if (madt_lapic_addr) {
//             lapic_address_physical = madt_lapic_addr;
//         }
//     } else if (mps_supported) {
//         if (mps_lapic_addr) {
//             lapic_address_physical = mps_lapic_addr;
//         }
//     } else {
//         //panic("APIC Init: This computer does not have an APIC!");
//     }
//     
//     /* Map to address space of HAL */
//     kernel_indirect_map(hal_per_processor_area_get_init() + PER_CPU_ARE_LAPIC_OFFSET,
//                         lapic_address_physical, 1);
//     
// }
// 
// void init_apic_mp()
// {
//     if (apic_supported) {
//         map_apic_mp();
//         init_lapic_mp();
//         init_lapic_timer_mp();
//     }
// }
// 
// static void map_apic()
// {
//     /* First we initlaize Local APIC for Bootstrap Processor */
//     kprintf("\tLocal APIC for Bootstrap Processor\n");
//     ulong lapic_address_physical = LAPIC_DEFAULT_PHYSICAL_ADDR;
//     
//     /* Get the physical address of LAPIC and IOAPIC from ACPI or MP Specification */
//     if (acpi_supported && madt_supported) {
//         if (madt_lapic_addr) {
//             lapic_address_physical = madt_lapic_addr;
//         }
//     } else if (mps_supported) {
//         if (mps_lapic_addr) {
//             lapic_address_physical = mps_lapic_addr;
//         }
//     } else {
//         //panic("APIC Init: This computer does not have an APIC!");
//     }
//     
//     /* Map to address space of HAL */
//     ulong lapic_virt_addr = PER_CPU_AREA_START + PER_CPU_ARE_LAPIC_OFFSET;
//     kprintf("\t\tPhysical Inuse %h, Default %h, Virtual %h\n",
//                 lapic_address_physical,
//                 LAPIC_DEFAULT_PHYSICAL_ADDR,
//                 lapic_virt_addr
//     );
//     kernel_indirect_map(lapic_virt_addr, lapic_address_physical, 1);
//     
//     /* Get APIC ID for BSP */
//     bsp_lapic_id = get_lapic_id();
//     
//     /* Next we initlaize IO APIC */
//     kprintf("\tIO APIC\n");
//     ulong ioapic_address_physical = IOAPIC_DEFAULT_PHYSICAL_ADDR;
//     ioapic_count = 8;
//     
//     /* Get the physical address of LAPIC and IOAPIC from ACPI or MP Specification */
//     if (acpi_supported && madt_supported) {
//         if (madt_ioapic_count < ioapic_count) {
//             ioapic_count = madt_ioapic_count;
//         }
//         
//         if (!ioapic_count) {
//             ioapic_count = 1;
//         }
//         
//         struct acpi_madt_ioapic *e = NULL;
//         int i = 0;
//         while (e = get_next_ioapic_entry(e, &ioapic_address_physical)) {
//             kernel_indirect_map(
//                 IOAPIC_START_VIRTUAL_ADDR + PAGE_SIZE * i++,
//                 ioapic_address_physical ? ioapic_address_physical : IOAPIC_DEFAULT_PHYSICAL_ADDR
//             );
//         }
//     }
//     
//     else if (mps_supported) {
//         if (mps_ioapic_count < ioapic_count) {
//             ioapic_count = mps_ioapic_count;
//         }
//         
//         if (!ioapic_count) {
//             ioapic_count = 1;
//         }
// 
//     }
//     
//     else {
//         //panic("APIC Init: This computer does not have an APIC!");
//     }
// }
// 
// void init_apic()
// {
//     kprintf("Initializing interupt controller\n");
//  
//     apic_present = cpu_apic_present();
//     
//     if (
//         ((acpi_supported && madt_supported) || mps_supported) &&
//         (apic_present)
//     ) {
//         kprintf("\tAPIC present and supported\n");
//         apic_supported = 1;
//         
//         /* Disable 8259A */
//         init_i8259a();
//         disable_i8259a();
//         
//         /* Initialize APIC */
//         map_apic();
//         
//         /* Enable Local APIC In BSP in order to start APs */
//         init_lapic_mp();
//         init_lapic_ipi();
//         
//         init_ioapic();
//     } else {
//         kprintf("\tAPIC not supported, use i8259a instead. MP is disabled!\n");
//         init_i8259a();
//     }
//     
// }


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
#include "hal/include/i8259a.h"


int apic_supported = 0;
int apic_present = 0;

int ioapic_count = 8;

ulong lapic_paddr = LAPIC_DEFAULT_PADDR;


static ulong cpu_apic_present()
{
    struct cpuid_reg reg;
    
    reg.a = 1;
    reg.b = 0;
    reg.c = 0;
    reg.d = 0;
    cpuid(&reg);
    
    return (reg.d & (0x1 << 9));
}

static void map_ioapic()
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

static void find_lapic_paddr()
{
    // Figure out LAPIC paddr
    if (acpi_supported && madt_supported) {
        if (madt_lapic_addr) {
            lapic_paddr = madt_lapic_addr;
        }
    } else if (mps_supported) {
        if (mps_lapic_addr) {
            lapic_paddr = mps_lapic_addr;
        }
    } else {
        panic("APIC Init: Unable to figure out LAPIC paddr!");
    }
    
    kprintf("\tLocal APIC physical addr: %p\n", lapic_paddr);
}

void init_apic()
{
    kprintf("Initializing interupt controller\n");
 
    apic_present = cpu_apic_present();

    if (
        ((acpi_supported && madt_supported) || mps_supported) &&
        (apic_present)
    ) {
        kprintf("\tAPIC present and supported\n");
        apic_supported = 1;
        
        // Disable 8259A
        init_i8259a();
        disable_i8259a();
        
        // Find out paddr of LAPIC
        find_lapic_paddr();
        
        // Map IOAPIC
        map_ioapic();
    } else {
        kprintf("\tAPIC not supported, use i8259a instead. MP is disabled!\n");
        init_i8259a();
    }
}
