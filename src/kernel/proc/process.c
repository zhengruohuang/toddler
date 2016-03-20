/*
 * Process manager
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"
#include "kernel/include/coreimg.h"
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
    p->name = strdup(name);
    p->url = strdup(url);
    
    p->type = type;
    p->state = process_enter;
    
    p->user_mode = type != process_kernel;
    
    p->priority = priority;
    
    // Thread list
    p->threads.count = 0;
    p->threads.next = NULL;
    
    // Page table
    if (type == process_kernel) {
        p->page_dir_pfn = hal->kernel_page_dir_pfn;
    } else {
        p->page_dir_pfn = palloc(1);
    }
    assert(p->page_dir_pfn);
    
    // Init the address space
    if (type != process_kernel) {
        hal->init_addr_space(p->page_dir_pfn);
    }
    
    // Init dynamic area
    create_dalloc(p);
    
    // Memory
    p->memory.entry_point = 0;
    
    p->memory.program_start = 0;
    p->memory.program_end = 0;
    
    p->memory.heap_start = 0;
    p->memory.heap_end = 0;
    
    if (type == process_kernel) {
        p->memory.dynamic_top = 0;
        p->memory.dynamic_bottom = 0;
    } else {
        p->memory.dynamic_top = p->dynamic.cur_top;
        p->memory.dynamic_bottom = p->dynamic.cur_top;
    }
    
    // Insert the process into process list
    p->prev = NULL;
    p->next = processes.next;
    processes.next = p;
    processes.count++;
    
    // Done
    return p;
}

int load_image(struct process *p, char *url)
{
    // Load image
    ulong img = (ulong)load_core_file(url); // FIXME: should use namespace service
    ulong entry = 0, vaddr_start = 0, vaddr_end = 0;
    int succeed = hal->load_exe(img, p->page_dir_pfn, &entry, &vaddr_start, &vaddr_end);
    
    if (!succeed) {
        return 0;
    }
    
    // Calculae rounded addresses
    ulong heap_start = vaddr_end;
    
    // Set memory layout
    p->memory.program_start = vaddr_start;
    p->memory.program_end = vaddr_end;
    p->memory.heap_start = heap_start;
    p->memory.heap_end = heap_start;
    
    return 1;
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
