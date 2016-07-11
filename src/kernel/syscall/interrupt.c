/*
 * Kernel interrupt handler
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"
#include "kernel/include/ds.h"


struct int_hdlr_record {
    struct process *process;
    unsigned long handler_entry;
};


static int interrupt_handler_record_salloc_id;
static hashtable_t interrupt_handlers;


/*
 * Initialize
 */
void init_interrupt()
{
    interrupt_handler_record_salloc_id = salloc_create(sizeof(struct int_hdlr_record), 0, 0, NULL, NULL);
    hashtable_create(&interrupt_handlers, 0, NULL);
    
    kprintf("Kernel interrupt handling initialized, interrupt record salloc ID: %d\n",
            interrupt_handler_record_salloc_id);
}


/*
 * Register interrupt
 */
void reg_interrupt(struct process *p, unsigned long irq, unsigned long thread_entry)
{
    // Allocate a record
    struct int_hdlr_record *record = (struct int_hdlr_record *)salloc(interrupt_handler_record_salloc_id);
    record->process = p;
    record->handler_entry = thread_entry;
    
    // Register the msg handler
    hashtable_insert(&interrupt_handlers, irq, record);
    
    kprintf("Interrupt handler registered, IRQ: %x, process: %s\n", irq, p->name);
}

void unreg_interrupt(struct process *p, unsigned long irq)
{
    struct int_hdlr_record *handler = (struct int_hdlr_record *)hashtable_obtain(&interrupt_handlers, irq);
    if (!handler) {
        return;
    }
    hashtable_release(&interrupt_handlers, irq, handler);
    
    // Unregister the msg handler
    hashtable_remove(&interrupt_handlers, irq);
    
    sfree(handler);
}


/*
 * Interrupt forward
 */
static void msg_param_value(msg_t *m, unsigned long value)
{
    int index = m->param_count;
    
    m->params[index].type = MSG_PARAM_VALUE;
    m->params[index].value = value;
    
    m->param_count++;
}

void interrupt_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the handler
    struct int_hdlr_record *handler = (struct int_hdlr_record *)hashtable_obtain(&interrupt_handlers, disp_info->interrupt.irq);
    
    kprintf("IRQ: %d, handler: %p\n", disp_info->interrupt.irq, handler);
    
    // Create a new handler thread
    if (handler) {
        struct thread *t;
        msg_t *m;
        
        assert(handler->process);
        assert(handler->handler_entry);
        
        //kprintf("Thread created, proc: %s, entry: %p\n", handler->process->name, handler->handler_entry);
        
        t = create_thread(handler->process, handler->handler_entry, 0, -1, 0, 0);
        set_thread_arg(t, t->memory.block_base + t->memory.msg_recv_offset);
        
        // Setup a message
        m = (msg_t *)t->memory.msg_recv_paddr;
        m->func_num = 0;
        m->mailbox_id = 0;
        m->msg_size = sizeof(msg_t);
        m->opcode = 0;
        m->param_count = 0;
        
        msg_param_value(m, disp_info->interrupt.irq);
        msg_param_value(m, disp_info->interrupt.vector);
        msg_param_value(m, disp_info->interrupt.param0);
        msg_param_value(m, disp_info->interrupt.param1);
        msg_param_value(m, disp_info->interrupt.param2);
        
        // Run the thread
        run_thread(t);
    }
    
    if (handler) {
        hashtable_release(&interrupt_handlers, disp_info->interrupt.irq, handler);
    }
}
