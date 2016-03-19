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
// static ulong has_apic()
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
//     apic_present = has_apic();
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
#include "hal/include/int.h"
#include "hal/include/apic.h"
#include "hal/include/i8259a.h"


int apic_supported = 0;
int apic_present = 0;


static ulong has_apic()
{
    struct cpuid_reg reg;
    
    reg.a = 1;
    reg.b = 0;
    reg.c = 0;
    reg.d = 0;
    cpuid(&reg);
    
    return (reg.d & (0x1 << 9));
}

void init_apic_mp()
{
    assert(has_apic() && apic_supported);
    
    init_lapic_mp();
}

void init_apic()
{
    kprintf("Initializing interupt controller\n");
 
    apic_present = has_apic();

    if (
        ((acpi_supported && madt_supported) || mps_supported) &&
        (apic_present)
    ) {
        kprintf("\tAPIC present and supported\n");
        apic_supported = 1;
        
        // Disable 8259A
        init_i8259a();
        disable_i8259a();
        
        // Init local APIC
        init_lapic();
        init_lapic_timer();
        init_ipi();
        
        // Map IOAPIC
        init_ioapic();
    } else {
        kprintf("\tAPIC not supported, use i8259a instead. MP is disabled!\n");
        init_i8259a();
    }
}

void start_working_mp()
{
    //kprintf("Start working MP!\n");
    //start_lapic_timer();
}

void start_working()
{
    start_lapic_timer();
    lapic_eoi();
    
    enable_local_int();
}
