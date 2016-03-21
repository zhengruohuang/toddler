#ifndef __ARCH_IA32_HAL_INCLUDE_SYSCALL__
#define __ARCH_IA32_HAL_INCLUDE_SYSCALL__


#include "common/include/data.h"
#include "common/include/task.h"


/*
 * C sysenter handler
 */
#define SYSENTER_MSR_CS     0x174
#define SYSENTER_MSR_ESP    0x175
#define SYSENTER_MSR_EIP    0x176

extern u32 syscall_proxy_entry;
extern void asmlinkage save_context_sysenter(struct context *context);
extern void asmlinkage sysenter_handler_entry();


/*
 * Assembly sysenter handler
 */
extern void sysenter_handler();
extern void sysenter_proxy_start_origin();
extern void sysenter_proxy_end_origin();


/*
 * General syscall
 */

void init_syscall_mp();
void init_syscall();


#endif
