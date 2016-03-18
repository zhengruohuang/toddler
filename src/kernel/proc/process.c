/*
 * Process manager
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


static int proc_salloc_id;

static struct process_list processes;
struct process *kernel_proc;


static ulong gen_proc_id(struct process *p)
{
    ulong id = (ulong)p;
    return id;
}


struct process *create_process(
    ulong parent_id, char *name, char *url,
    enum process_type type, int priority)
{
    // Allocate a process struct
    struct process *p = (struct process *)salloc(proc_salloc_id);
    assert(p);
    
    // Assign a proc id
    p->proc_id = gen_proc_id(p);
    p->parent_id = parent_id;
    
    // Setup the process
    p->name = name; // FIXME: need to duplicate the name instead of a simple assign
    p->url = url;   // FIXME: so does url
    
    p->type = type;
    p->state = process_enter;
    
    p->user_mode = type != process_kernel;
    
    p->priority = priority;
    
    // Thread list
    p->threads.count = 0;
    p->threads.next = NULL;
    
    // Page table
    p->page_dir_pfn = palloc(1);
    assert(p->page_dir_pfn);
    
    // Memory
    p->memory.entry_point = 0;
    
    p->memory.program_start = 0;
    p->memory.program_end = 0;
    
    p->memory.dynamic_top = 0;
    p->memory.dynamic_bottom = 0;
    
    p->memory.heap_start = 0;
    p->memory.heap_end = 0;
    
    // Insert the process into process list
    p->prev = NULL;
    p->next = processes.next;
    processes.next = p;
    processes.count++;
    
    // Done
    return p;
}


void init_process()
{
    kprintf("Initializing process manager\n");
    
    // Create salloc obj
    proc_salloc_id = salloc_create(sizeof(struct process), 0, 0, NULL, NULL);
    
    // Init process list
    processes.count = 0;
    processes.next = NULL;
    
    // Create the kernel process
    kernel_proc = create_process(-1, "kernel", "coreimg://tdlrkrnl.bin", process_kernel, 0);
    
    kprintf("\tSalloc ID: %d, Kernel process ID: %x\n", proc_salloc_id, kernel_proc->proc_id);
}
