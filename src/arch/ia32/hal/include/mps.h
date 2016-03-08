#ifndef __ARCH_IA32_HAL_INCLUDE_MPS__
#define __ARCH_IA32_HAL_INCLUDE_MPS__


#include "common/include/data.h"


#define MPS_FPS_SIGNATURE            (0x5f504d5f)
#define MPS_CT_SIGNATURE             (0x504d4350)

#define MPS_CT_EXT_ENTRY_TYPE        0
#define MPS_CT_EXT_ENTRY_LEN         1


struct mps_fps {
    u32     signature;
    u32     configuration_table;
    u8      length;
    u8      revision;
    u8      checksum;
    u8      config_type;
    u8      mpfib2;
    u8      mpfib3;
    u8      mpfib4;
    u8      mpfib5;
} packedstruct;

struct mps_ct {
    u32     signature;
    u16     base_table_length;
    u8      revision;
    u8      checksum;
    u8      oem_id[8];
    u8      product_id[12];
    u32     oem_table;
    u16     oem_table_size;
    u16     entry_count;
    u32     lapic_address;
    u16     ext_table_length;
    u8      ext_table_checksum;
    u8      reserved;
    u8      base_table[0];
} packedstruct;


struct mps_processor {
    u8      type;
    u8      lapic_id;
    u8      lapic_version;
    union {
        u8      cpu_flags;
        struct {
            u8      cpu_flags_enabled       : 1;
            u8      cpu_flags_bootstrap     : 1;
            u8      cpu_flags_reserved      : 6;
        };
    };
    union {
        u32     cpu_signature;
        struct {
            u32     cpu_signature_stepping  : 4;
            u32     cpu_signature_model     : 4;
            u32     cpu_signature_family    : 4;
            u32     cpu_signature_reserved  : 20;
        };
    };
    union {
        u32     feature_flags;
        struct {
            u32     feature_flags_fpu       : 1;
            u32     feature_flags_reserved1 : 6;
            u32     feature_flags_mce       : 1;
            u32     feature_flags_cx8       : 1;
            u32     feature_flags_apic      : 1;
            u32     feature_flags_reserved2 : 22;
        };
    };
    u32     reserved[2];
} packedstruct;

struct mps_bus {
    u8      type;
    u8      bus_id;
    u8      bus_type[6];
} packedstruct;

struct mps_ioapic {
    u8      type;
    u8      ioapic_id;
    u8      ioapic_version;
    u8      ioapic_flags;
    u32     ioapic_address;
} packedstruct;

struct mps_ioint {
    u8              type;
    u8              interrupt_type;
    union {
        u8          flags;
        struct {
            u8      polarity        : 2;
            u8      trigger         : 2;
            u8      reserved_1      : 4;
        };
    };
    u8              reserved_2;
    u8              src_bus_id;
    u8              src_bus_irq;
    u8              dest_ioapic_id;
    u8              dest_ioapic_pin;
} packedstruct;

struct mps_lintr {
    u8              type;
    u8              interrupt_type;
    union {
        u8          flags;
        struct {
            u8      polarity        : 2;
            u8      trigger         : 2;
            u8      reserved_1      : 4;
        };
    };
    u8              reserved_2;
    u8              src_bus_id;
    u8              src_bus_irq;
    u8              dest_lapic_id;
    u8              dest_lapic_pin;
} packedstruct;


extern int mps_supported;
extern ulong mps_lapic_addr;

extern struct mps_ioapic *get_next_mps_ioapic_entry(struct mps_ioapic *cur, ulong *ioapic_addr);

extern int mps_lapic_count;
extern int mps_ioapic_count;
extern int mps_bus_count;
extern int mps_ioint_count;
extern int mps_lint_count;

extern void init_mps();


#endif
