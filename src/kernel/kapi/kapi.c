/*
 * KAPI Handling
 */
#include "common/include/syscall.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/kapi.h"


hashtable_t kapi_servers;


msg_t *create_response_msg(struct thread *t)
{
    msg_t *m = (msg_t *)t->memory.msg_recv_paddr;
    
    m->func_num = 0;
    m->mailbox_id = 0;
    m->msg_size = sizeof(msg_t);
    m->opcode = 0;
    m->param_count = 0;
    
    return m;
}

void set_msg_param_value(msg_t *m, unsigned long value)
{
    int index = m->param_count;
    
    m->params[index].type = MSG_PARAM_VALUE;
    m->params[index].value = value;
    
    m->param_count++;
}


static void register_kapi(ulong kapi_num, kernel_msg_handler_t handler)
{
    hashtable_insert(&kapi_servers, kapi_num, (void *)kernel_proc);
    hashtable_insert(&kernel_proc->msg_handlers, kapi_num, (void *)handler);
}

void init_kapi()
{
    hashtable_create(&kapi_servers, 0, NULL);
    
    // Interrupt
    register_kapi(KAPI_INTERRUPT_REG, reg_interrupt_handler);
    register_kapi(KAPI_INTERRUPT_UNREG, unreg_interrupt_handler);
    
    // Thread
    register_kapi(KAPI_THREAD_CREATE, thread_create_handler);
    register_kapi(KAPI_THREAD_EXIT, thread_exit_handler);
    
    // Process
    register_kapi(KAPI_PROCESS_STARTED, process_started_handler);
    register_kapi(KAPI_PROCESS_EXIT, process_exit_handler);
    
    // Heap
    register_kapi(KAPI_HEAP_END_GET, get_heap_end_handler);
    register_kapi(KAPI_HEAP_END_SET, set_heap_end_handler);
    register_kapi(KAPI_HEAP_END_GROW, grow_heap_handler);
    register_kapi(KAPI_HEAP_END_SHRINK, shrink_heap_handler);
    
    kprintf("KAPI Initialized\n");
}
