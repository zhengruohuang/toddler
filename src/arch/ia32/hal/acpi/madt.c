#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/acpi.h"


int madt_supported = 0;
ulong madt_lapic_addr = 0;
int madt_lapic_count = 0;
int madt_ioapic_count = 0;
int madt_int_count = 0;

static struct acpi_madt *acpi_madt;


struct acpi_madt_lapic *get_next_lapic_entry(struct acpi_madt_lapic *cur, int *usable)
{
    struct acpi_madt_apic_header *end = (struct acpi_madt_apic_header *)(((ulong)acpi_madt) + acpi_madt->header.length);
    struct acpi_madt_apic_header *ptr;
    
    for (ptr = acpi_madt->apic_header; ptr < end; ptr = (struct acpi_madt_apic_header*)((ulong)ptr + ptr->length)) {
        if ((ulong)ptr > (ulong)cur && ptr->type == ACPI_MADT_LAPIC) {
            struct acpi_madt_lapic *ret = (struct acpi_madt_lapic *)ptr;
            if (usable) {
                *usable = ret->flags & 0x1;
            }
            return ret;
        }
    }
    
    return NULL;
}

struct acpi_madt_ioapic *get_next_ioapic_entry(struct acpi_madt_ioapic *cur, int *ioapic_addr)
{
    struct acpi_madt_apic_header *end = (struct acpi_madt_apic_header *)(((ulong)acpi_madt) + acpi_madt->header.length);
    struct acpi_madt_apic_header *ptr;
    
    for (ptr = acpi_madt->apic_header; ptr < end; ptr = (struct acpi_madt_apic_header*)((ulong)ptr + ptr->length)) {
        if ((ulong)ptr > (ulong)cur && ptr->type == ACPI_MADT_IOAPIC) {
            struct acpi_madt_ioapic *ret = (struct acpi_madt_ioapic *)ptr;
            if (ioapic_addr) {
                *ioapic_addr = ret->ioapic_address;
            }
            return ret;
        }
    }
    
    return NULL;
}

struct acpi_madt_intr_src_ovrd *get_next_int_entry(struct acpi_madt_intr_src_ovrd *cur, int *src, int *gint, int *pol, int *tri)
{
    struct acpi_madt_apic_header *end = (struct acpi_madt_apic_header *)(((ulong)acpi_madt) + acpi_madt->header.length);
    struct acpi_madt_apic_header *ptr;
    
    for (ptr = acpi_madt->apic_header; ptr < end; ptr = (struct acpi_madt_apic_header*)((ulong)ptr + ptr->length)) {
        if ((ulong)ptr > (ulong)cur && ptr->type == ACPI_MADT_IOAPIC) {
            struct acpi_madt_intr_src_ovrd *ret = (struct acpi_madt_intr_src_ovrd *)ptr;
            if (src)    *src = ret->source;
            if (gint)   *gint = ret->global_int;
            if (pol)    *pol = ret->polarity;
            if (tri)    *tri = ret->trigger;
            return ret;
        }
    }
    
    return NULL;
}


static void lapic(struct acpi_madt_lapic *lapic, u32 i)
{
    kprintf("\t\tLocal APIC entry #%d at %p", i, lapic);
    
    if (!(lapic->flags & 0x1)) {
        kprintf(", Unusable!\n");
    } else {
        kprintf("\n");
    }
}

static void ioapic(struct acpi_madt_ioapic *ioapic, u32 i)
{
    kprintf("\t\tIO APIC entry #%d at %p: Map Address %h\n", i, ioapic, ioapic->ioapic_address);
}

static void interrupt(struct acpi_madt_intr_src_ovrd *override, u32 i)
{
    kprintf("\t\tInterrupt entry #%d at %p: Src %d, Int %d, Pol %d, Tri %d\n",
               i, override,
               override->source, override->global_int, override->polarity, override->trigger
    );
}

int init_madt(struct acpi_madt *madt)
{
    acpi_madt = madt;
    
    // Map the table to HAL's address space
    kernel_direct_map((ulong)madt, 0);
    kernel_direct_map_array((ulong)madt, madt->header.length, 0);
    
    // Checksum
    if (acpi_byte_checksum(madt, madt->header.length)) {
        kprintf("\t\tMADT: Checksum Failed!");
        return 0;
    }
    
    // Scan all entries
    struct acpi_madt_apic_header *end = (struct acpi_madt_apic_header *)(((ulong)acpi_madt) + acpi_madt->header.length);
    struct acpi_madt_apic_header *ptr;
    
    // Echo MADT entries
    for (ptr = acpi_madt->apic_header; ptr < end; ptr = (struct acpi_madt_apic_header *)((ulong)ptr + ptr->length)) {
        switch (ptr->type) {
        case ACPI_MADT_LAPIC:
            lapic((struct acpi_madt_lapic *)ptr, madt_lapic_count++);
            break;
        case ACPI_MADT_IOAPIC:
            ioapic((struct acpi_madt_ioapic *)ptr, madt_ioapic_count++);
            break;
        case ACPI_MADT_INTR_SRC_OVRD:
            interrupt((struct acpi_madt_intr_src_ovrd *)ptr, madt_int_count++);
            break;
        case ACPI_MADT_NMI_SRC:
        case ACPI_MADT_LAPIC_NMI:
        case ACPI_MADT_LAPIC_ADDR_OVRD:
        case ACPI_MADT_IO_SAPIC:
        case ACPI_MADT_L_SAPIC:
        case ACPI_MADT_PLATFORM_INTR_SRC:
        default:
            break;
        }
    }
    
    // Local APIC addr
    madt_lapic_addr = (u32)acpi_madt->lapic_address;
    kprintf("\t\tLocal APIC address: %p\n", madt_lapic_addr);
    
    // Summary
    kprintf("\t\tLocal APIC count: %d, IO APIC count: %d, Interrupt count: %d\n",
        madt_lapic_count, madt_ioapic_count, madt_int_count
    );
    
    madt_supported = 1;
    return 1;
}
