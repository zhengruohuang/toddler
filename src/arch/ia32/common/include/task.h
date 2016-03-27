#ifndef __ARCH_IA32_COMMON_INCLUDE_CONTEXT__
#define __ARCH_IA32_COMMON_INCLUDE_CONTEXT__


#include "common/include/data.h"


/*
 * Context
 */
struct context {
    // Manually saved/stored
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;
    
    // pushad/popad
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp; // This field has no use, and POPAD will ignore it. It is PUSHAD that saves it automatically
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    
    // Pushed by interrupt handler
    u32 vector;
    u32 error_code;
    
    // Pushed by HW upon an interrupt
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
} packedstruct;


/*
 * Kernel dispatch info 
 */
enum kernel_dispatch_type {
    kdisp_unknown,
    kdisp_syscall,
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
            ulong param;
            
            // Filled by kernel
            void *worker;
        } syscall;
    };
    
    struct context *context;
    
    /*
     * Filled by kernel
     */
    void *proc;
    void *thread;
    void *sched;
};


#endif
