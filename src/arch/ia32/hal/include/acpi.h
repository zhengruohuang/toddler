#ifndef __ARCH_IA32_HAL_INCLUDE_ACPI__
#define __ARCH_IA32_HAL_INCLUDE_ACPI__


#include "common/include/data.h"


#define ACPI_RSDP_SIGNATURE             "RSD PTR "
#define ACPI_RSDP_SIGNATURE_LENGTH      8
#define ACPI_RSDP_REVISION_OFFS         15

#define ACPI_MADT_SIGNATURE             "APIC"
#define ACPI_FADT_SIGNATURE             "FACP"
#define ACPI_SRAT_SIGNATURE             "SRAT"
#define ACPI_SLIT_SIGNATURE             "SLIT"

#define ACPI_CMP_SIGNATURE(left, right)     \
            (((left)[0] == (right)[0]) &&   \
            ((left)[1] == (right)[1]) &&    \
            ((left)[2] == (right)[2]) &&    \
            ((left)[3] == (right)[3]))


#define ACPI_MADT_LAPIC                     0
#define ACPI_MADT_IOAPIC                    1
#define ACPI_MADT_INTR_SRC_OVRD             2
#define ACPI_MADT_NMI_SRC                   3
#define ACPI_MADT_LAPIC_NMI                 4
#define ACPI_MADT_LAPIC_ADDR_OVRD           5
#define ACPI_MADT_IO_SAPIC                  6
#define ACPI_MADT_L_SAPIC                   7
#define ACPI_MADT_PLATFORM_INTR_SRC         8
#define ACPI_MADT_RESERVED_SKIP_BEGIN       9
#define ACPI_MADT_RESERVED_SKIP_END         127
#define ACPI_MADT_RESERVED_OEM_BEGIN        128


/* Root System Description Pointer */
struct acpi_rsdp_v1 {
    /* RSDP v1 */
    u8                      signature[8];
    u8                      checksum;
    u8                      oem_id[6];
    u8                      revision;
    u32                     rsdt_address;
} packedstruct ;

struct acpi_rsdp {
    /* RSDP v1 */
    u8                      signature[8];
    u8                      checksum;
    u8                      oem_id[6];
    u8                      revision;
    u32                     rsdt_address;
    
    /* RSDP v2 */
    u32                     length;
    u64                     xsdt_address;
    u32                     ext_checksum;
    u8                      reserved[3];
} packedstruct;

/* System Description Table Header */
struct acpi_sdt_header {
    u8                      signature[4];
    u32                     length;
    u8                      revision;
    u8                      checksum;
    u8                      oem_id[6];
    u8                      oem_table_id[8];
    u32                     oem_revision;
    u32                     creator_id;
    u32                     creator_revision;
} packedstruct;

struct acpi_signature_map {
    u8                      signature;
    struct acpi_sdt_header  **sdt_ptr;
    u8                      description;
    int                     found;
};
/******************************************************************************/


/*******************************************************************************
 *                                 RSDT/XSDT *
 ******************************************************************************/
/* Root System Description Table */
struct acpi_rsdt {
    struct acpi_sdt_header  header;
    u32                     entry[];
} packedstruct;

/* Extended System Description Table */
struct acpi_xsdt {
    struct acpi_sdt_header  header;
    u64                     entry[];
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 * MADT
 ******************************************************************************/
/* Multiple APIC Description Table Header */
struct acpi_madt_apic_header{
    u8                              type;
    u8                              length;
} packedstruct;

/* Multiple APIC Description Table */
struct acpi_madt {
    struct acpi_sdt_header          header;
    u32                             lapic_address;
    u32                             flags;
    struct acpi_madt_apic_header    apic_header[];
} packedstruct;

struct acpi_madt_lapic {
    struct acpi_madt_apic_header    header;
    u8                              acpi_id;
    u8                              apic_id;
    union {
        u32                         flags;
        struct {
            u32                     flags_enabled   : 1;
            u32                     flags_reserved  : 31;
        };
    };
} packedstruct;

struct acpi_madt_ioapic {
    struct acpi_madt_apic_header    header;
    u8                              ioapic_id;
    u8                              reserved;
    u32                             ioapic_address;
    u32                             global_intr_base;
} packedstruct;

struct acpi_madt_intr_src_ovrd {
    struct acpi_madt_apic_header    header;
    u8                              bus;
    u8                              source;
    u32                             global_int;
    union {
        u16                         flags;
        
        struct {
            u16                     polarity        : 2;
            u16                     trigger         : 2;
            u16                     reserved        : 12;
        };
    };
} packedstruct;

struct acpi_madt_nmi_src {
    struct acpi_madt_apic_header    header;
    u16                             flags;
    u32                             global_intr;
} packedstruct;

struct acpi_madt_lapic_nmi {
    struct acpi_madt_apic_header    header;
    u8                              acpi_id;
    u16                             flags;
    u8                              lapic_lint;
} packedstruct;

struct acpi_madt_lapic_addr_ovrd {
    struct acpi_madt_apic_header    header;
    u16                             reserved;
    u64                             lapic_address;
} packedstruct;

struct acpi_madt_io_sapic {
    struct acpi_madt_apic_header    header;
    u8                              ioapic_id;
    u8                              reserved;
    u32                             global_intr_base;
    u64                             ioapic_address;
} packedstruct;

struct acpi_madt_l_sapic {
    struct acpi_madt_apic_header    header;
    u8                              acpi_id;
    u8                              sapic_id;
    u8                              sapic_eid;
    u8                              reserved[3];
    u32                             flags;
    u32                             acpi_processor_uid_value;
    u8                              acpi_processor_uid_str[1];
} packedstruct;

struct acpi_madt_platform_intr_src {
    struct acpi_madt_apic_header    header;
    u16                             flags;
    u8                              intr_type;
    u8                              processor_id;
    u8                              processor_eid;
    u8                              io_sapic_vector;
    u32                             global_intr;
    u32                             platform_intr_src_flags;
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 * FADT
 ******************************************************************************/
struct acpi_fadt {
    struct acpi_sdt_header          header;
    u32                             facs_address;
    u32                             dsdt_address;
    u8                              reserved_fields1[20];
    u32                             pm1a;
    u8                              reserved_fields2[64];
    u64                             xfacs_address;
    u64                             xdsdt_address;
    u8                              reserved_fields3[96];
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 * DSDT
 ******************************************************************************/
struct acpi_dsdt {
    struct acpi_sdt_header          header;
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 * 
 * SLIT - System Locality Distance Information Table
 *        Version 1
 *
 ******************************************************************************/
struct acpi_slit {
    struct acpi_sdt_header          header;                 /* Common ACPI table header */
    u64                             locality_count;
    u8                              entries[];              /* Real size = localities^2 */
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 * ACPI Shutdown
 ******************************************************************************/
enum {
    ACPI_OPCODE_ZERO = 0,
    ACPI_OPCODE_ONE = 1,
    ACPI_OPCODE_NAME = 8,
    ACPI_OPCODE_BYTE_CONST = 0x0a,
    ACPI_OPCODE_WORD_CONST = 0x0b,
    ACPI_OPCODE_DWORD_CONST = 0x0c,
    ACPI_OPCODE_STRING_CONST = 0x0d,
    ACPI_OPCODE_SCOPE = 0x10,
    ACPI_OPCODE_BUFFER = 0x11,
    ACPI_OPCODE_PACKAGE = 0x12,
    ACPI_OPCODE_METHOD = 0x14,
    ACPI_OPCODE_EXTOP = 0x5b,
    ACPI_OPCODE_CREATE_WORD_FIELD = 0x8b,
    ACPI_OPCODE_CREATE_BYTE_FIELD = 0x8c,
    ACPI_OPCODE_IF = 0xa0,
    ACPI_OPCODE_ONES = 0xff
};

enum {
    ACPI_EXTOPCODE_MUTEX = 0x01,
    ACPI_EXTOPCODE_OPERATION_REGION = 0x80,
    ACPI_EXTOPCODE_FIELD_OP = 0x81,
    ACPI_EXTOPCODE_INDEX_FIELD_OP = 0x86,
};

#define ACPI_SLP_EN (1 << 13)
#define ACPI_SLP_TYP_OFFSET 10
/******************************************************************************/


/*
 * MADT
 */
extern int madt_supported;
extern ulong madt_lapic_addr;
extern int madt_lapic_count;
extern int madt_ioapic_count;
extern int madt_int_count;

extern int init_madt(struct acpi_madt *madt);


/*
 * FADT
 */
extern int fadt_supported;
extern int acpi_shutdown_supported;

extern int init_fadt(struct acpi_fadt *fadt);


/*
 * ACPI
 */
extern int acpi_supported;
extern int acpi_v2_enabled;

extern int acpi_byte_checksum(void *base, u32 size);
extern void init_acpi();


#endif
