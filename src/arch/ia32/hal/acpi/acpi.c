#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/acpi.h"


int acpi_supported = 0;
int acpi_v2_enabled = 0;


static struct acpi_rsdp *acpi_rsdp;
static struct acpi_rsdt *acpi_rsdt;
static struct acpi_xsdt *acpi_xsdt;


int acpi_byte_checksum(void *base, u32 size)
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
    u8 *ebda_phys_addr = (u8 *)get_bootparam()->ebda_addr;
    u8 *ptr;
    
    kprintf("\t\tSearching for RSDP v2 in EBDA ... ");
    
    for (ptr = ebda_phys_addr; ptr < ebda_phys_addr + 0x400; ptr += 16) {
        if (
            memcmp(ptr, ACPI_RSDP_SIGNATURE, ACPI_RSDP_SIGNATURE_LENGTH) == 0 &&
            acpi_byte_checksum(ptr, sizeof(struct acpi_rsdp_v1)) == 0 &&
            ((struct acpi_rsdp *)ptr)->revision != 0 &&
            ((struct acpi_rsdp *)ptr)->length < 1024 &&
            acpi_byte_checksum(ptr, ((struct acpi_rsdp *)ptr)->length) == 0
        ) {
            acpi_rsdp = (struct acpi_rsdp *)ptr;
            acpi_v2_enabled = 1;
            return 1;
        }
    }
    
    kprintf("not found!\n");
    return 0;
}

static int scan_v2_bios()
{
    u8 *ptr;
    
    kprintf("\t\tSearching for RSDP v2 in BIOS ... ");
    
    for (ptr = (u8 *)0xe0000; ptr < (u8 *)0x100000; ptr += 16) {
        if (
            memcmp(ptr, ACPI_RSDP_SIGNATURE, ACPI_RSDP_SIGNATURE_LENGTH) == 0 &&
            acpi_byte_checksum(ptr, sizeof(struct acpi_rsdp_v1)) == 0 &&
            ((struct acpi_rsdp *)ptr)->revision != 0 &&
            ((struct acpi_rsdp *)ptr)->length < 1024 &&
            acpi_byte_checksum(ptr, ((struct acpi_rsdp *)ptr)->length) == 0
        ) {
            acpi_rsdp = (struct acpi_rsdp *)ptr;
            acpi_v2_enabled = 1;
            return 1;
        }
    }
    
    kprintf("not found!\n");
    return 0;
}

static int scan_v1_ebda()
{
    u8 *ebda_phys_addr = (u8 *)get_bootparam()->ebda_addr;
    u8 *ptr;
    
    kprintf("\t\tSearching for RSDP v1 in EBDA ... ");

    for (ptr = ebda_phys_addr; ptr < ebda_phys_addr + 0x400; ptr += 16) {
        if (
            memcmp (ptr, "RSD PTR ", 8) == 0 &&
            acpi_byte_checksum(ptr, sizeof(struct acpi_rsdp_v1)) == 0 &&
            ((struct acpi_rsdp_v1 *)ptr)->revision == 0
        ) {
            acpi_rsdp = (struct acpi_rsdp *)ptr;
            return 1;
        }
    }
    
    kprintf("not found!\n");
    return 0;
}

static int scan_v1_bios()
{
    u8 *ptr;
    kprintf("\t\tSearching for RSDP v1 in BIOS ... ");
    
    for (ptr = (u8 *)0xe0000; ptr < (u8 *)0x100000; ptr += 16) {
        if (memcmp(ptr, "RSD PTR ", 8) == 0 &&
            acpi_byte_checksum(ptr, sizeof(struct acpi_rsdp_v1)) == 0 &&
            ((struct acpi_rsdp_v1*)ptr)->revision == 0
        ) {
            acpi_rsdp = (struct acpi_rsdp *)ptr;
            return 1;
        }
    }
    
    kprintf("not found!\n");
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
        kprintf("found at %p!\n", acpi_rsdp);
    }
    
    else if (scan_v2_bios()) {
        acpi_supported = 1;
        kprintf("found at %p!\n", acpi_rsdp);
    }
    
    else if (scan_v1_ebda()) {
        acpi_supported = 1;
        kprintf("found at %p!\n", acpi_rsdp);
    }
    
    else if (scan_v1_bios()) {
        acpi_supported = 1;
        kprintf("found at %p!\n", acpi_rsdp);
    }
    
    else {
        acpi_supported = 0;
        kprintf("not found!\n");
    }
    
    return acpi_supported;
}

void init_acpi()
{
    kprintf("Initializing ACPI\n");
    
    // Find RSDP
    find_rsdp();
    if (!acpi_supported) {
        kprintf("\tACPI not supported!\n");
        return;
    }
    
    // Get and map RSDT or XSDT
    if (acpi_v2_enabled) {
        acpi_xsdt = (struct acpi_xsdt *)(acpi_rsdp->xsdt_address);
        kernel_direct_map((ulong)acpi_xsdt, 0);
    } else {
        acpi_rsdt = (struct acpi_rsdt *)(acpi_rsdp->rsdt_address);
        kernel_direct_map((ulong)acpi_rsdt, 0);
    }
    
    // Find out all tables
    kprintf("\tScanning All ACPI Tables\n");
    
    int count = 0;
    if (acpi_v2_enabled) {
        count = (acpi_xsdt->header.length - sizeof(struct acpi_sdt_header)) / sizeof(u64);
    } else {
        count = (acpi_rsdt->header.length - sizeof(struct acpi_sdt_header)) / sizeof(u32);
    }
    
    int i;
    for (i = 0; i < count; i++) {
        struct acpi_sdt_header *hdr;
        
        if (acpi_v2_enabled) {
            hdr = (struct acpi_sdt_header *)acpi_xsdt->entry[i];
        } else {
            hdr = (struct acpi_sdt_header *)acpi_rsdt->entry[i];
        }
        
        // Map the table
        kernel_direct_map((ulong)hdr, 0);
        
        // MADT
        if (!memcmp(hdr->signature, ACPI_MADT_SIGNATURE, 4)) {
            kprintf("\tFound MADT at %p\n", hdr);
            init_madt((struct acpi_madt *)hdr);
        }
        
        // FADT
        //else if (!memcmp(hdr->signature, ACPI_FADT_SIGNATURE, 4)) {
        //    kprintf("\tFound FADT at %p\n", hdr);
        //    init_fadt((struct acpi_fadt *)hdr);
        //}
    }
}
