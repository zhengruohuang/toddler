#ifndef __ARCH_ARMV7_COMMON_INCLUDE_REG__
#define __ARCH_ARMV7_COMMON_INCLUDE_REG__


#include "common/include/data.h"


/*
 * Generic write to Coprocessor
 */
#define __mcr(value, coproc, op1, op2, n, m)                                \
    __asm__ __volatile__ (                                                  \
        "mcr " #coproc ", " #op1 ", %[r], " #n ", " #m ", " #op2 ";"        \
        :                                                                   \
        : [r] "r" (value)                                                   \
        : "memory"                                                          \
    )

#define __mcr2(value, coproc, op1, op2, n, m)                               \
    __asm__ __volatile__ (                                                  \
        "mcr2 " #coproc ", " #op1 ", %[r], " n ", " m ", " #op2 ";"         \
        :                                                                   \
        : [r] "r" (value)                                                   \
        : "memory"                                                          \
    )

#define __mcrr(value1, value2, coproc, op1, m)                              \
    __asm__ __volatile__ (                                                  \
        "mcrr " #coproc ", " #op1 ", %[r1], %[r2], " #m ";"                 \
        :                                                                   \
        : [r1] "r" (value1), [r2] "r" (value2)                              \
        : "memory"                                                          \
    )

#define __mcrr2(value1, value2, coproc, op1, m)                             \
    __asm__ __volatile__ (                                                  \
        "mcrr2 " #coproc ", " #op1 ", %[r1], %[r2], " #m ";"                \
        :                                                                   \
        : [r1] "r" (value1), [r2] "r" (value2)                              \
        : "memory"                                                          \
    )



/*
 * Generic read from Coprocessor
 */
#define __mrc(value, coproc, op1, op2, n, m)                                \
    __asm__ __volatile__ (                                                  \
        "mrc " #coproc ", " #op1 ", %[r], " #n ", " #m ", " #op2 ";"        \
        : [r] "=r" (value)                                                  \
        :                                                                   \
        : "memory", "cc"                                                    \
    )

#define __mrc2(value, coproc, op1, op2, n, m)                               \
    __asm__ __volatile__ (                                                  \
        "mrc2 " #coproc ", " #op1 ", %[r], " #n ", " #m ", " #op2 ";"       \
        : [r] "=r" (value)                                                  \
        :                                                                   \
        : "memory", "cc"                                                    \
    )

#define __mrrc(value1, value2, coproc, op1, m)                              \
    __asm__ __volatile__ (                                                  \
        "mrrc " #coproc ", " #op1 ", %[r1], %[r2], " #m ";"                 \
        : [r1] "=r" (value1), [r2] "=r" (value2)                            \
        :                                                                   \
        : "memory", "cc"                                                    \
    )

#define __mrrc2(value1, value2, coproc, op1, m)                             \
    __asm__ __volatile__ (                                                  \
        "mrrc2 " #coproc ", " #op1 ", %[r1], %[r2], " #m ";"                \
        : [r1] "=r" (value1), [r2] "=r" (value2)                            \
        :                                                                   \
        : "memory", "cc"                                                    \
    )


/*
 * Generic PSR read/write
 */
#define __mrs(value, which)             \
    __asm__ __volatile__ (              \
        "mrs %[r], " #which ";"         \
        : [r] "=r" (value)              \
        :                               \
        : "memory"                      \
    )

#define __msr(value, which)             \
    __asm__ __volatile__ (              \
        "msr " #which ", %[r];"         \
        :                               \
        : [r] "r" (value)               \
        : "memory", "cc"                \
    )


/*
 * Address translation
 */
struct trans_tab_base_reg {
    union {
        u32 value;
        
        struct {
            u32 walk_inner_cacheable    : 1;
            u32 walk_inner_shared       : 1;
            u32 reserved1               : 1;
            u32 walk_outer_attributes   : 2;
            u32 reserved2               : 27;
        };
        
        u32 base_addr;
    };
};

struct domain_access_ctrl_reg {
    union {
        u32 value;
        
        struct {
            u32 domain0     : 2;    // 00: no access
            u32 domain1     : 2;    // 01: client, accesses are checked
            u32 domain2     : 2;    // 10: reserved, any access generates a domain fault
            u32 domain3     : 2;    // 11: manager, accesses are not checked
            u32 domain4     : 2;
            u32 domain5     : 2;
            u32 domain6     : 2;
            u32 domain7     : 2;
            u32 domain8     : 2;
            u32 domain9     : 2;
            u32 domain10    : 2;
            u32 domain11    : 2;
            u32 domain12    : 2;
            u32 domain13    : 2;
            u32 domain14    : 2;
            u32 domain15    : 2;
        };
    };
};

#define read_trans_tab_base0(value)     __mrc(value, p15, 0, 0, c2, c0)
#define write_trans_tab_base0(value)    __mcr(value, p15, 0, 0, c2, c0)

#define read_trans_tab_base1(value)     __mrc(value, p15, 0, 1, c2, c0)
#define write_trans_tab_base1(value)    __mcr(value, p15, 0, 1, c2, c0)

#define read_trans_tab_base_ctrl(value) __mrc(value, p15, 0, 2, c2, c0)
#define write_trans_tab_base_ctrl(value) __mcr(value, p15, 0, 2, c2, c0)

#define read_domain_access_ctrl(value)  __mrc(value, p15, 0, 0, c3, c0)
#define write_domain_access_ctrl(value) __mcr(value, p15, 0, 0, c3, c0)


/*
 * System control
 */
struct sys_ctrl_reg {
    union {
        u32 value;
        
        struct {
            u32 mmu_enabled     : 1;
            u32 strict_align    : 1;
            u32 dcache_enabled  : 1;
            u32 reserved1       : 7;
            u32 swap_enabled    : 1;
            u32 bpred_enabled   : 1;
            u32 icache_enabled  : 1;
            u32 high_except_vec : 1;
            u32 rr_replacement  : 1;
            u32 reserved2       : 10;
            u32 reserved3       : 7;
        };
    };
};

#define read_sys_ctrl(value)    __mrc(value, p15, 0, 0, c1, c0)
#define write_sys_ctrl(value)   __mcr(value, p15, 0, 0, c1, c0)


/*
 * Software thread ID
 */
#define read_software_thread_id(value)  __mrc(value, p15, 0, 3, c13, c0)
#define write_software_thread_id(value) __mcr(value, p15, 0, 3, c13, c0)


/*
 * TLB
 */
#define inv_tlb_all()       __mcr(0, p15, 0, 0, c8, c7)
#define inv_tlb_addr(addr)  __mcr(0, p15, 0, 1, c8, c7)
#define inv_tlb_asid(asid)  __mcr(0, p15, 0, 2, c8, c7)



/*
 * Processor status
 */
struct proc_status_reg {
    union {
        u32 value;
        
        struct {
            u32 mode            : 5;
            u32 thumb           : 1;
            u32 fiq_mask        : 1;
            u32 irq_mask        : 1;
            u32 async_mask      : 1;
            u32 big_endian      : 1;
            u32 if_then_high    : 6;
            u32 greater_equal   : 4;
            u32 reserved        : 4;
            u32 jazelle         : 1;
            u32 if_then_low     : 2;
            u32 saturation      : 1;
            u32 overflow        : 1;
            u32 carry           : 1;
            u32 zero            : 1;
            u32 negative        : 1;
        };
    };
};

#define read_current_proc_status(value)     __mrs(value, CPSR)
#define write_current_proc_status(value)    __msr(value, CPSR)

#define read_saved_proc_status(value)       __mrs(value, SPSR)
#define write_saved_proc_status(value)      __msr(value, SPSR)


/*
 * Generic timer
 */
#define read_generic_timer_freq(value)      __mrc(value, p15, 0, 0, c14, c0)

#define read_generic_timer_phys_ctrl(value)     __mrc(value, p15, 0, 1, c14, c2)
#define write_generic_timer_phys_ctrl(value)    __mcr(value, p15, 0, 1, c14, c2)

#define read_generic_timer_phys_interval(value)  __mrc(value, p15, 0, 0, c14, c2)
#define write_generic_timer_phys_interval(value) __mcr(value, p15, 0, 0, c14, c2)

#define read_generic_timer_phys_compare(hi, lo)  __mrrc(lo, hi, p15, 2, c14)
#define write_generic_timer_phys_compare(hi, lo) __mcrr(lo, hi, p15, 2, c14)

#endif
