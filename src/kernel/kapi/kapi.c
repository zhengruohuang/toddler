/*
 * KAPI Handling
 */
#include "common/include/syscall.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/kapi.h"


hashtable_t kapi_servers;


static void register_kapi(ulong kapi_num, kernel_msg_handler_t handler)
{
    hashtable_insert(&kapi_servers, kapi_num, (void *)kernel_proc);
    hashtable_insert(&kernel_proc->msg_handlers, kapi_num, (void *)handler);
}

void init_kapi()
{
    hashtable_create(&kapi_servers, 0, NULL);
    
    register_kapi(KAPI_THREAD_EXIT, thread_exit_handler);
    register_kapi(KAPI_INTERRUPT_REG, reg_interrupt_handler);
    register_kapi(KAPI_INTERRUPT_UNREG, unreg_interrupt_handler);
    
    kprintf("KAPI Initialized\n");
}
