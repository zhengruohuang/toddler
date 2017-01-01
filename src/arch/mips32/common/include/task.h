#ifndef __ARCH_MIPS32_COMMON_INCLUDE_CONTEXT__
#define __ARCH_MIPS32_COMMON_INCLUDE_CONTEXT__


#include "common/include/data.h"


/*
 * Context
 */
struct context {
    u32 zero;
    u32 at;
    u32 v0, v1;
    u32 a0, a1, a2, a3;
    u32 t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    u32 s0, s1, s2, s3, s4, s5, s6, s7;
    u32 k0, k1;
    u32 gp;
    u32 sp;
    u32 fp;
    u32 ra;
    
    u32 pc;
    u32 delay_slot;
} packedstruct;


/*
 * Kernel dispatch info 
 */
enum kernel_dispatch_type {
    kdisp_unknown,
    kdisp_syscall,
    kdisp_interrupt,
    kdisp_exception,
    
    kdisp_page_fault,
    kdisp_protection,
    kdisp_illegal_instr,
};

struct kernel_dispatch_info {
    /*
     * Filled by HAL
     */
    enum kernel_dispatch_type dispatch_type;
    
    union {
        struct {
            // Filled by HAL
            ulong num;
            ulong param0;
            ulong param1;
            ulong param2;
            
            // Filled by kernel syscall handler
            ulong return0;
            ulong return1;
        } syscall;
        
        struct {
            // Filled by HAL
            ulong vector;
            ulong irq;
            
            ulong param0;
            ulong param1;
            ulong param2;
        } interrupt;
    };
    
    // Filled by HAL
    struct context *context;
    
    // Filled by kernel
    void *worker;
    
    /*
     * Filled by kernel
     */
    void *proc;
    void *thread;
    void *sched;
};


#endif
