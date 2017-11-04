#ifndef __ARCH_MIPS64_COMMON_INCLUDE_REG__
#define __ARCH_MIPS64_COMMON_INCLUDE_REG__


#include "common/include/data.h"


/*
 * Generic read/write
 */
#define __mfc0(value, reg, sel)             \
    __asm__ __volatile__ (                  \
        "mfc0 %[r], $" #reg ", " #sel ";"   \
        : [r] "=r" (value)                  \
        :                                   \
    )

#define __mtc0(value, reg, sel)             \
    __asm__ __volatile__ (                  \
        "mtc0 %[r], $" #reg ", " #sel ";"   \
        :                                   \
        : [r] "r" (value)                   \
    )

#define __dmfc0(value, reg, sel)            \
    __asm__ __volatile__ (                  \
        "dmfc0 %[r], $" #reg ", " #sel ";"  \
        : [r] "=r" (value)                  \
        :                                   \
    )

#define __dmtc0(value, reg, sel)            \
    __asm__ __volatile__ (                  \
        "dmtc0 %[r], $" #reg ", " #sel ";"  \
        :                                   \
        : [r] "r" (value)                   \
    )


/*
 * Status
 */
struct cp0_status {
    union {
        struct {
            u32 ie      : 1;
            u32 exl     : 1;
            u32 erl     : 1;
            u32 ksu     : 2;
            u32 ux      : 1;
            u32 sx      : 1;
            u32 kx      : 1;
            u32 im      : 8;
            u32 mid     : 2;
            u32 zero1   : 1;
            u32 nmi     : 1;
            u32 sr      : 1;
            u32 zero2   : 1;
            u32 bev     : 1;
            u32 px      : 1;
            u32 mx      : 1;
            u32 zero3   : 1;
            u32 fr      : 1;
            u32 rp      : 1;
            u32 cu0     : 1;
            u32 cu1     : 1;
            u32 cu2     : 1;
            u32 zero4   : 1;
        };
        
        u32 value;
    };
};

#define read_cp0_status(value)   __mfc0(value, 12, 0)
#define write_cp0_status(value)  __mtc0(value, 12, 0)


/*
 * Exception
 */
struct cp0_ebase {
    union {
        struct {
            u64 cpunum      : 10;
            u64 zero0       : 1;
            u64 write_gate  : 1;
            u64 base        : 52;
        };
        
        u64 value;
    };
};

#define read_cp0_ebase(value)   __dmfc0(value, 15, 1)
#define write_cp0_ebase(value)  __dmtc0(value, 15, 1)


/*
 * TLB
 */
struct cp0_entry_hi {
    union {
        struct {
            u64 asid    : 8;
            u64 asid_ex : 2;
            u64 hw_inv  : 1;
            u64 vpn2_ex : 2;
            u64 vpn2    : 49;
            u64 region  : 2;
        };
        
        u64 value;
    };
} packedstruct;

struct cp0_page_mask {
    union {
        struct {
            u64 zero0   : 11;
            u64 mask_ex : 2;
            u64 mask    : 47;
            u64 zero1   : 4;
        };
        
        u64 value;
    };
} packedstruct;

struct cp0_entry_lo {
    union {
        struct {
            u64 global  : 1;
            u64 valid   : 1;
            u64 dirty   : 1;
            u64 coherent: 3;
            u64 pfn     : 49;
            u64 fill    : 7;
            u64 xi      : 1;
            u64 ri      : 1;
        };
        
        u64 value;
    };
} packedstruct;

struct cp0_page_grain {
    union {
        struct {
            u32 mc_cause    : 5;
            u32 zero0       : 3;
            u32 ase         : 5;
            u32 zero1       : 13;
            u32 s32x        : 1;
            u32 iec         : 1;
            u32 esp         : 1;
            u32 elpa        : 1;
            u32 xie         : 1;
            u32 rie         : 1;
        };
        
        u32 value;
    };
} packedstruct;

#define read_cp0_entry_hi(value)    __dmfc0(value, 10, 0)
#define write_cp0_entry_hi(value)   __dmtc0(value, 10, 0)

#define read_cp0_page_mask(value)   __dmfc0(value, 5, 0)
#define write_cp0_page_mask(value)  __dmtc0(value, 5, 0)

#define read_cp0_entry_lo0(value)   __dmfc0(value, 2, 0)
#define write_cp0_entry_lo0(value)  __dmtc0(value, 2, 0)

#define read_cp0_entry_lo1(value)   __dmfc0(value, 3, 0)
#define write_cp0_entry_lo1(value)  __dmtc0(value, 3, 0)

#define read_cp0_page_grain(value)  __mfc0(value, 5, 1)
#define write_cp0_page_grain(value) __mtc0(value, 5, 1)

#define read_cp0_index(value)       __mfc0(value, 0, 0)
#define write_cp0_index(value)      __mtc0(value, 0, 0)

#define read_cp0_random(value)      __mfc0(value, 1, 0)

#define read_cp0_bad_vaddr(value)   __dmfc0(value, 8, 0)


/*
 * Interrupt
 */
struct cp0_cause {
    union {
        struct {
            u32 zero0       : 2;
            u32 exc_code    : 5;
            u32 zero1       : 1;
            u32 ip          : 8;
            u32 ase         : 2;
            u32 zero2       : 3;
            u32 fdci        : 1;
            u32 wp          : 1;
            u32 iv          : 1;
            u32 ase2        : 2;
            u32 pci         : 1;
            u32 dc          : 1;
            u32 ce          : 2;
            u32 ti          : 1;
            u32 bd          : 1;
        };
        
        u32 value;
    };
} packedstruct;

#define read_cp0_cause(value)   __mfc0(value, 13, 0)
#define write_cp0_cause(value)  __mtc0(value, 13, 0)

#define read_cp0_epc(value)     __dmfc0(value, 14, 0)
#define write_cp0_epc(value)    __dmtc0(value, 14, 0)


/*
 * Processor ID
 */
struct cp0_proc_id {
    union {
        struct {
            u32 rev     : 8;
            u32 proc_id : 8;
            u32 comp_id : 8;
            u32 comp_opt: 8;
        };
        
        u32 value;
    };
} packedstruct;

#define read_cp0_proc_id(value)   __mfc0(value, 15, 0)


/*
 * Config
 */
struct cp0_config {
    union {
        struct {
            u32 info        : 31;
            u32 has_next    : 1;
        };
        
        u32 value;
    };
};

struct cp0_config0 {
    union {
        struct {
            u32 kseg0       : 3;
            u32 virt_icache : 1;
            u32 zero0       : 3;
            u32 mmu_type    : 3;
            u32 arch_rev    : 3;
            u32 arch_type   : 2;
            u32 big_endian  : 1;
            u32 zero1       : 9;
            u32 fixed_kuseg : 3;
            u32 fixed_k23   : 3;
            u32 has_config1 : 1;
        };
        
        u32 value;
    };
} packedstruct;

struct cp0_config1 {
    union {
        struct {
            u32 has_fpu     : 1;
            u32 has_ejtag   : 1;
            u32 has_mips16  : 1;
            u32 has_watch   : 1;
            u32 has_perf    : 1;
            u32 has_mdmx    : 1;
            u32 has_cp2     : 1;
            u32 dcache_assoc: 3;
            u32 dcache_line : 3;
            u32 dcache_sets : 3;
            u32 icache_assoc: 3;
            u32 icache_line : 3;
            u32 icache_sets : 3;
            u32 vtlb_size   : 6;
            u32 has_config2 : 1;
        };
        
        u32 value;
    };
} packedstruct;

struct cp0_config2 {
    union {
        struct {
            u32 l2_assoc    : 4;
            u32 l2_line     : 4;
            u32 l2_sets     : 4;
            u32 l2_status   : 4;
            u32 l3_assoc    : 4;
            u32 l3_line     : 4;
            u32 l3_sets     : 4;
            u32 l3_status   : 3;
            u32 has_config3 : 1;
        };
        
        u32 value;
    };
} packedstruct;

struct cp0_config3 {
    union {
        struct {
            u32 reserved    : 30;
            u32 has_big_page: 1;
            u32 has_config4 : 1;
        };
        
        u32 value;
    };
} packedstruct;

struct cp0_config4 {
    union {
        struct {
            union {
                struct {
                    u32 ftlb_sets   : 4;
                    u32 ftlb_ways   : 4;
                };
                u32 mmu_size_ext    : 8;
            };
            u32 ftlb_page   : 5;
            u32 zero        : 1;
            u32 mmu_ext_type: 2;
            u32 kscr_map    : 8;
            u32 vtlb_size_ex: 4;
            u32 has_asid_ex : 1;
            u32 has_hw_inv  : 2;
            u32 has_config5 : 1;
        };
        
        u32 value;
    };
} packedstruct;

struct cp0_config5 {
    union {
        struct {
            u32 has_nested_fault: 1;
            u32 has_ufr         : 1;
            u32 has_mattri_regs : 1;
            u32 has_ll_bit      : 1;
            u32 has_mvcp_high   : 1;
            u32 kern_only_sdbbp : 1;
            u32 has_virt_proc   : 1;
            u32 ena_fre         : 1;
            u32 ena_ufe         : 1;
            u32 no_config2      : 1;
            u32 has_dual_endian : 1;
            u32 zero0           : 1;
            u32 no_wide_llsc    : 1;
            u32 zero1           : 14;
            u32 ena_simd        : 1;
            u32 has_eva         : 1;
            u32 dis_cache_vec   : 1;
            u32 dis_seg_ctrl    : 1;
            u32 has_config6     : 1;
        };
        
        u32 value;
    };
} packedstruct;

#define read_cp0_config(idx, value)     __mfc0(value, 16, idx)
#define write_cp0_config(idx, value)    __mtc0(value, 16, idx)

#define write_cp0_wired(value)          __mtc0(value, 6, 0)


#endif
