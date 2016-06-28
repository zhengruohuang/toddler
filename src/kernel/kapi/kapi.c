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
    
    kprintf("KAPI Initialized\n");
}


asmlinkage void thread_exit_handler(struct kernel_msg_handler_arg *arg)
{
    kprintf("To terminate user thread: %p\n", arg->sender_thread);
    
    terminate_thread(arg->sender_thread);
    terminate_thread(arg->handler_thread);
    
    sfree(arg);
    
    kprintf("Threads should have been terminated\n");
    
    // Wait for this thread to be terminated
    do {
        hal->sleep();
    } while (1);
    
    // Should never reach here
    kprintf("kapi.c: Should never reach here!\n");
    do {
        hal->sleep();
    } while (1);
}
