#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/bios.h"
#include "hal/include/acpi.h"


static struct acpi_rsdp *acpi_rsdp;
static struct acpi_rsdt *acpi_rsdt;
static struct acpi_xsdt *acpi_xsdt;
static enum acpi_table_type acpi_table_type;
int acpi_supported = 0;


static int byte_checksum(d_ptr base, u32 size)
{
    u8 *ptr;
    u8 ret = 0;
    
    for (ptr = (u8 *)base; ptr < ((u8 *)base) + size; ptr++) {
        ret += *ptr;
    }
    
    return (int)ret;
}

static int scan_v2_ebda()
{
    kprintf("\tSearching for RSDP v2 in EBDA ... ");
    
    u8 *ptr;
    for (ptr = (u8 *)ebda_phys_addr; ptr < (u8 *)(ebda_phys_addr + 0x400); ptr += 16) {
        if (
            memcmp(ptr, ACPI_RSDP_SIGNATURE, ACPI_RSDP_SIGNATURE_LENGTH) == 0 &&
            byte_checksum(ptr, sizeof(struct acpi_rsdp_v1)) == 0 &&
            ((struct acpi_rsdp *)ptr)->revision != 0 &&
            ((struct acpi_rsdp *)ptr)->length < 1024 &&
            byte_checksum(ptr, ((struct acpi_rsdp *)ptr)->length) == 0
        ) {
            kprintf("found at %p\n", ptr);
            acpi_rsdp = (struct acpi_rsdp *)ptr;
            acpi_table_type == acpi_table_type_xsdt;
            return 1;
        }
    }
    
    return 0;
}

static int scan_v2_bios()
{
    u8 *ptr;
    kprintf("\tSearching for RSDP v2 in BIOS ... ");
    
    for (ptr = (u8*)0xe0000; ptr < (u8*)0x100000; ptr += 16) {
        if (
            memcmp(ptr, ACPI_RSDP_SIGNATURE, ACPI_RSDP_SIGNATURE_LENGTH) == 0 &&
            byte_checksum(ptr, sizeof(struct acpi_rsdp_v1)) == 0 &&
            ((struct acpi_rsdp *)ptr)->revision != 0 &&
            ((struct acpi_rsdp *)ptr)->length < 1024 &&
            byte_checksum(ptr, ((struct acpi_rsdp *)ptr)->length) == 0
        ) {
            kprintf("Found at %p\n", ptr);
            acpi_rsdp = (struct acpi_rsdp *)ptr;
            acpi_table_type == acpi_table_type_xsdt;
            return 1;
        }
    }
    
    return 0;
}

static int scan_v1_ebda()
{
    u8 *ptr;
    kprintf("\tSearching for RSDP v1 in EBDA ... ");

    for (ptr = (u8 *)ebda_phys_addr; ptr < (u8 *)(ebda_phys_addr + 0x400); ptr += 16) {
        if (
            memcmp (ptr, "RSD PTR ", 8) == 0 &&
            byte_checksum(ptr, sizeof(struct acpi_rsdp_v1)) == 0 &&
            ((struct acpi_rsdp_v1 *)ptr)->revision == 0
        ) {
            kprintf("Found at %p\n", ptr);
            acpi_rsdp = (struct acpi_rsdp *)ptr;
            acpi_table_type == acpi_table_type_rsdt;
            return 1;
        }
    }
    
    return 0;
}

static int scan_v1_bios()
{
    u8 *ptr;
    kprintf("\tSearching for RSDP v1 in BIOS ... ");
    
    for (ptr = (u8*)0xe0000; ptr < (u8*)0x100000; ptr += 16) {
        if (memcmp(ptr, "RSD PTR ", 8) == 0 &&
            byte_checksum(ptr, sizeof(struct acpi_rsdp_v1)) == 0 &&
            ((struct acpi_rsdp_v1*)ptr)->revision == 0
        ) {
            kprintf("Found at %p\n", ptr);
            acpi_rsdp = (struct acpi_rsdp *)ptr;
            acpi_table_type == acpi_table_type_rsdt;
            return 1;
        }
    }
    
    return 0;
}

static int find_rsdp()
{
    /*
     * Find Root System Description Pointer
     * 1. search first 1K of EBDA
     * 2. search 128K starting at 0xe0000
     */
    
    // Look for RSDT
    kprintf("\tSearching for Root System Description Pointer (RSDP)\n");
    
    if (scan_v2_ebda()) {
        acpi_supported = 1;
        kprintf("found at %p\n", acpi_rsdp);
    }
    
    else if (scan_v2_bios()) {
        kprintf("not found!\n");
        acpi_supported = 1;
        kprintf("found at %p\n", acpi_rsdp);
    }
    
    else if (scan_v1_ebda()) {
        kprintf("not found!\n");
        acpi_supported = 1;
        kprintf("found at %p\n", acpi_rsdp);
    }
    
    else if (scan_v1_bios()) {
        kprintf("not found!\n");
        acpi_supported = 1;
        kprintf("found at %p\n", acpi_rsdp);
    }
    
    else {
        kprintf("not found!\n");
        acpi_supported = 0;
    }
}
