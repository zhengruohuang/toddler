#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "common/include/task.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/acpi.h"
#include "hal/include/mps.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/apic.h"


struct apic_irq_to_pin {
    u8              chip;
    u8              pin;
    union {
        u8          vector;
        u8          irq;
    };
    
    union {
        u8          flags;
        
        struct {
            u8      trigger         : 1;
            u8      polarity        : 1;
            u8      available       : 1;
        };
    };
};


static struct apic_irq_to_pin irq_map[256];
static struct apic_irq_to_pin vector_map[256];

int ioapic_count = 8;


static int ioapic_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    //kprintf("IO APIC Interrupt Vector %d, IRQ %d\n", vector_map[vector_num].vector, vector_map[vector_num].irq);
    
    u32 result = 1;
    
    // Dissable IRQ
    //ioapic_disable_irq(vector_map[vector_num].irq);
    
    switch (vector_map[context->vector].irq) {
        case 0:
            //result = hal_interrupt_handler_global_timer();
            break;
        case 1:
            //result = hal_interrupt_handler_keyboard();
            break;
        default:
            break;
    }
    
    // Enable IRQ
    lapic_eoi();
    ioapic_enable_irq(vector_map[context->vector].irq);
    
    
    return result;
}

static void ioapic_override_madt()
{
    int i, j;
    
    int irq = 0, vector = 0, polarity = 0, trigger = 0;
    struct acpi_madt_intr_src_ovrd *int_entry = NULL;
    
    
    for (i = 0; i < madt_int_count; i++) {
        int_entry = get_next_acpi_int_entry(int_entry, &irq, &vector, &polarity, &trigger);
        assert(int_entry);

        irq_map[irq].vector = vector;
        struct acpi_madt_ioapic* ioapic_entry = NULL;
        
        // We only support 1 IO APIC... hehe
        assert(madt_ioapic_count == 1);
        
        // Determine IO APIC Chip Number and Pin Number
        for (j = 0; j < madt_ioapic_count; j++) {
            ioapic_entry = get_next_acpi_ioapic_entry(ioapic_entry, NULL);
            assert(ioapic_entry);
            
            // If this IO APIC's global interrupt base is smaller than current global interrupt
            if (vector >= ioapic_entry->global_intr_base) {
                irq_map[irq].chip = j;
                irq_map[irq].pin = vector - ioapic_entry->global_intr_base;
                irq_map[irq].available = 1;
            } else {
                panic("\tUnable to setup IO APIC!");
            }
        }
        
        // Disable default entry which has the same chip and pin number with current interrupt
        for (j = 0; j < 256; j++) {
            if (
                j != irq &&
                irq_map[j].chip == irq_map[irq].chip &&
                irq_map[j].pin == irq_map[irq].pin
            ) {
                irq_map[j].available = 0;
            }
        }
        
        // Polarity
        switch (polarity) {
            case 1:
                irq_map[irq].polarity = APIC_POLARITY_HIGH;
                break;
            case 3:
                irq_map[irq].polarity = APIC_POLARITY_LOW;
                break;
            case 2:
                panic("\tUnsupported Polarlity!");
                break;
            default:
                break;
        }
        
        // Trigger Mode
        switch (trigger) {
            case 1:
                irq_map[irq].trigger = APIC_TRIGMOD_EDGE;
                break;
            case 3:
                irq_map[irq].trigger = APIC_TRIGMOD_LEVEL;
                break;
            case 2:
                panic("\tUnsupported Trigger Mode!");
                break;
            default:
                break;
        }
    }
}

static void ioapic_override_mps()
{
    int i, j;
    
    int irq = 0, polarity = 0, trigger = 0, pin = 0, chip = 0, bus = 0;
    struct mps_ioint *entry = NULL;
    
    // We only support 1 IO APIC
    assert(mps_ioapic_count == 1);
    
    for (i = 0; i < mps_ioint_count; i++) {
        entry = get_next_mps_ioint_entry(entry, &bus, &irq, &chip, &pin, &polarity, &trigger);
        assert(entry);
        
        if (entry->interrupt_type) {
            continue;
        }
        
        struct mps_ioapic *ioapic_entry = NULL;
        
        // Determine IO APIC Chip Number, and Pin Number
        for (j = 0; j < mps_ioapic_count; j++) {
            ioapic_entry = get_next_mps_ioapic_entry(ioapic_entry, NULL);
            assert(ioapic_entry);
            
            // Check this IO APIC's ID
            if (ioapic_entry->ioapic_id == entry->dest_ioapic_id) {
                irq_map[irq].chip = j;
                irq_map[irq].pin = entry->dest_ioapic_pin;
                irq_map[irq].available = 1;
                
                break;
            }
        }
        
        // Disable default entry which has the same chip and pin number with current interrupt
        for (j = 0; j < 256; j++) {
            if (
                j != irq &&
                irq_map[j].chip == irq_map[irq].chip &&
                irq_map[j].pin == irq_map[irq].pin
            ) {
                irq_map[j].available = 0;
            }
        }
        
        // Polarity
        switch (polarity) {
            case 1:
                irq_map[irq].polarity = APIC_POLARITY_HIGH;
                break;
            case 3:
                irq_map[irq].polarity = APIC_POLARITY_LOW;
                break;
            case 2:
                panic("Unsupported Polarlity!");
                break;
            default:
                break;
        }
        
        // Trigger Mode
        switch (trigger) {
            case 1:
                irq_map[irq].trigger = APIC_TRIGMOD_EDGE;
                break;
            case 3:
                irq_map[irq].trigger = APIC_TRIGMOD_LEVEL;
                break;
            case 2:
                panic("Unsupported Trigger Mode!");
                break;
            default:
                break;
        }
    }
}

void ioapic_start()
{
    ioapic_enable_irq(0);
    ioapic_enable_irq(1);
}

void ioapic_init_redirection_table(int chip, int pin, int vector, int trigger, int polarity)
{
    struct io_redirection_register reg;
    reg.low = ioapic_read(chip, APIC_IO_REDTBL + pin * 2);
    reg.high = ioapic_read(chip, APIC_IO_REDTBL + pin * 2 + 1);
    
    reg.destmod = APIC_DESTMOD_PHYS;
    reg.delmod = APIC_DELMOD_FIXED;
    reg.trigger_mode = trigger;
    reg.intpol = polarity;
    reg.vector = vector;
    reg.masked = 1;
    
    reg.dest = 0; //0xff;
    
    ioapic_write(chip, APIC_IO_REDTBL + pin * 2, reg.low);
    ioapic_write(chip, APIC_IO_REDTBL + pin * 2 + 1, reg.high);
}

void ioapic_change_io_redirection_table(int chip, int pin, int dest, int vec, u32 flags)
{
    u32 dlvr;
    
    if (flags & APIC_LOPRI) {
        dlvr = APIC_DELMOD_LOWPRI;
    }
    else {
        dlvr = APIC_DELMOD_FIXED;
    }
    
    struct io_redirection_register reg;
    reg.low = ioapic_read(chip, APIC_IO_REDTBL + pin * 2);
    reg.high = ioapic_read(chip, APIC_IO_REDTBL + pin * 2 + 1);
    
    reg.dest = dest;
    reg.destmod = APIC_DESTMOD_PHYS;
    reg.trigger_mode = APIC_TRIGMOD_EDGE;
    reg.intpol = APIC_POLARITY_HIGH;
    reg.delmod = dlvr;
    reg.vector = vec;
    
    ioapic_write(chip, APIC_IO_REDTBL + pin * 2, reg.low);
    ioapic_write(chip, APIC_IO_REDTBL + pin * 2 + 1, reg.high);
}

void ioapic_disable_irq(u16 irq)
{
    struct apic_irq_to_pin pin_record = irq_map[irq];
    u8 pin = pin_record.pin;
    u8 chip = pin_record.chip;
    
    if (pin != -1 && chip != -1) {
        struct io_redirection_register reg;
        
        reg.low = ioapic_read(chip, APIC_IO_REDTBL + pin * 2);
        reg.masked = 1;
        ioapic_write(chip, APIC_IO_REDTBL + pin * 2, reg.low);
    }
}

void ioapic_enable_irq(u16 irq)
{
    struct apic_irq_to_pin pin_record = irq_map[irq];
    u8 pin = pin_record.pin;
    u8 chip = pin_record.chip;
    
    if (pin != -1 && chip != -1) {
        struct io_redirection_register reg;
        
        reg.low = ioapic_read(chip, APIC_IO_REDTBL + pin * 2);
        reg.masked = 0;
        ioapic_write(chip, APIC_IO_REDTBL + pin * 2, reg.low);
    }
}

u32 *ioapic_get_chip_base(int chip)
{
    assert(chip < ioapic_count);
    
    u32 result = IOAPIC_TOP_VADDR - PAGE_SIZE * (chip + 1);
    
    return (u32*)result;
}

u32 ioapic_read(int chip, int address)
{
    struct apic_io_regseclect_register regsel;
    u32* chip_base = ioapic_get_chip_base(chip);
    
    regsel.value = chip_base[APIC_IO_REGSEL];
    regsel.reg_addr = address;
    chip_base[APIC_IO_REGSEL] = regsel.value;
    
    u32 result = chip_base[APIC_IO_WIN];
    
    return result;
}

void ioapic_write(int chip, int address, u32 val)
{
    struct apic_io_regseclect_register regsel;
    u32* chip_base = ioapic_get_chip_base(chip);
    
    regsel.value = chip_base[APIC_IO_REGSEL];
    regsel.reg_addr = address;
    chip_base[APIC_IO_REGSEL] = regsel.value;
    
    chip_base[APIC_IO_WIN] = val;
}


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
        while ((e = get_next_acpi_ioapic_entry(e, &ioapic_paddr)) != NULL) {
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
        while ((e = get_next_mps_ioapic_entry(e, &ioapic_paddr)) != NULL) {
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
    //panic("get_bootparam()->hal_vspace_end: %p\n", get_bootparam()->hal_vspace_end);
    
    // Override IO APIC using MADT or MPS
    int i;
    
    // Set default ISA IRQ map
    for (i = 0; i < 16; i++) {
        irq_map[i].chip = 0;
        irq_map[i].pin = i;
        irq_map[i].vector = i;
        irq_map[i].available = 1;
        
        // Default ISA Trigger Mode: EDGE (As ACPI Specification 5.0)
        irq_map[i].trigger = APIC_TRIGMOD_EDGE;
        
        // Default ISA Polarity: Active High (I guess)
        irq_map[i].polarity = APIC_POLARITY_HIGH;
    }
    
    if (acpi_supported && madt_supported) {
        // Go through MADT Interrupt Structures
        ioapic_override_madt();
    } else if (mps_supported) {
        // Go through MP IO Interrupt Structures
        ioapic_override_mps();
    }
    
    kprintf("\tIO APIC IRQ map\n");
    
    for (i = 0; i < 16; i++) {
        if (!irq_map[i].available) {
            continue;
        }
        
        kprintf("\t\tIRQ: %d, Vector: %d, Chip: %d, Pin: %d, Trigger: %d, Polarity: %d\n",
                i,
                irq_map[i].vector,
                irq_map[i].chip,
                irq_map[i].pin,
                irq_map[i].trigger,
                irq_map[i].polarity
        );
    }
    
    kprintf("\tIO APIC vector map\n");
    
    // Initialize Vector Map
    for (i = 0; i < 256; i++) {
        vector_map[i].available = 0;
    }
    
    // Generate Vector Map, and Program Redirection Table
    for (i = 0; i < 256; i++) {
        if (!irq_map[i].available) {
            continue;
        }
        
        int vector = alloc_int_vector(ioapic_handler);
        vector_map[vector].irq = i;
        
        vector_map[vector].chip = irq_map[i].chip;
        vector_map[vector].pin = irq_map[i].pin;
        vector_map[vector].available = irq_map[i].available;
        vector_map[vector].trigger = irq_map[i].trigger;
        vector_map[vector].polarity = irq_map[i].polarity;
        
        // Redirection Table
        ioapic_init_redirection_table(
            vector_map[vector].chip,
            vector_map[vector].pin,
            vector,
            vector_map[vector].trigger,
            vector_map[vector].polarity
        );
        
        kprintf("\t\tVector: %d, IRQ: %d, Chip: %d, Pin: %d, Trigger: %d, Polarity: %d\n",
                vector,
                vector_map[vector].vector,
                vector_map[vector].chip,
                vector_map[vector].pin,
                vector_map[vector].trigger,
                vector_map[vector].polarity
        );
    }
}
