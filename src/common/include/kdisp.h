#ifndef __COMMON_INCLUDE_KDISPATCH__
#define __COMMON_INCLUDE_KDISPATCH__


#include "common/include/data.h"
#include "common/include/context.h"


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
