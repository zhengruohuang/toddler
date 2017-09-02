#include "common/include/data.h"
#include "common/include/proc.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/assert.h"
#include "klibc/include/sys.h"


/*
 * Gets called before the kernel starts the process creation procedure
 * Return value indicates if the process is allowed to be created
 */
static asmlinkage void on_process_create_before(msg_t *msg)
{
    unsigned long ret_mbox_id = msg->mailbox_id;
    int errno = EOK;
    
    kprintf("Before process creation!\n");
    
    // Setup the reply msg
    msg_t *r = syscall_msg();
    r->mailbox_id = ret_mbox_id;
    msg_param_value(r, (unsigned long)errno);
    
    // Issue the response
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}

/*
 * Gets called after the process has been successfully created
 *  or on a failure of process creation
 * While before any of the process' thread starts running
 * Return value indicates if the process is allowed to run
 *  Negative values will laed to termination of the process
*/
static asmlinkage void on_process_create_after(msg_t *msg)
{
    unsigned long ret_mbox_id = msg->mailbox_id;
    int errno = EOK;
    
    kprintf("After process creation!\n");
    
    // Setup the reply msg
    msg_t *r = syscall_msg();
    r->mailbox_id = ret_mbox_id;
    msg_param_value(r, (unsigned long)errno);
    
    // Issue the response
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}

/*
 * Gets called after all of the process' threads have been suspended
 *  While before any clean-up procedure
 * Return value indicates if the process should be cleaned-up
 *  Negative values will lead to the process being resumed
 */
static asmlinkage void on_process_terminate_before(msg_t *msg)
{
    unsigned long ret_mbox_id = msg->mailbox_id;
    int errno = EOK;
    
    // Setup the reply msg
    msg_t *r = syscall_msg();
    r->mailbox_id = ret_mbox_id;
    msg_param_value(r, (unsigned long)errno);
    
    // Issue the response
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}

/*
 * Gets called after kernel-mode resource clean-up is finished
 *  while before the PID is free to be reused.
 * Return value indicates if user-mode clean-up has been successfully finished
 *  Negative values will lead to a system crash - failed to terminate process
 */
static asmlinkage void on_process_terminate_after(msg_t *msg)
{
    unsigned long ret_mbox_id = msg->mailbox_id;
    int errno = EOK;
    
    // Setup the reply msg
    msg_t *r = syscall_msg();
    r->mailbox_id = ret_mbox_id;
    msg_param_value(r, (unsigned long)errno);
    
    // Issue the response
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}


/*
 * Init
 */
#define reg_monitor(type, handler) do {                         \
    unsigned long func_num = alloc_msg_num();                   \
    kapi_process_monitor(type, func_num, (unsigned long)type);  \
    syscall_reg_msg_handler(func_num, handler);                 \
} while (0)

void init_process_monitor()
{
    reg_monitor(pm_create_before, on_process_create_before);
    reg_monitor(pm_create_after, on_process_create_after);
    reg_monitor(pm_terminate_before, on_process_terminate_before);
    reg_monitor(pm_terminate_after, on_process_terminate_after);
}
