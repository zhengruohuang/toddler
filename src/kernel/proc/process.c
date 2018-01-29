/*
 * Process manager
 */


#include "common/include/data.h"
#include "common/include/errno.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"
#include "kernel/include/coreimg.h"
#include "kernel/include/exec.h"
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
    int mon_err = EOK;
    struct process *p = NULL;
    
    // Notify process monitor
    mon_err = check_process_create_before(parent_id);
    assert(mon_err == EOK);
    
    // Allocate a process struct
    p = (struct process *)salloc(proc_salloc_id);
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
    create_thread_lists(p);
    
    // Page table
    if (type == process_kernel) {
        p->page_dir_pfn = hal->kernel_page_dir_pfn;
    } else {
        p->page_dir_pfn = palloc(hal->user_page_dir_page_count);
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
    
    // Msg handlers
    hashtable_create(&p->msg_handlers, 0, NULL, NULL);
    
    // ASID
    if (type == process_kernel) {
        p->asid = 0;
    } else {
        p->asid = asid_alloc();
    }
    
    // Insert the process into process list
    p->prev = NULL;
    p->next = processes.next;
    processes.next = p;
    processes.count++;
    
    // Notify process monitor
    mon_err = check_process_create_after(parent_id, p->proc_id);
    assert(mon_err == EOK);
    
    // Create cleaning thread
    if (type != process_kernel) {
        struct thread *t = create_thread(kernel_proc, (ulong)&kernel_tclean_thread, (ulong)p, -1, 0, 0);
        run_thread(t);
    }
    
    // Done
    return p;
}

int load_image(struct process *p, char *url)
{
    // Load image
    ulong img = (ulong)get_core_file_addr_by_name(url); // FIXME: should use namespace service
    ulong entry = 0, vaddr_start = 0, vaddr_end = 0;
    //int succeed = hal->load_exe(img, p->page_dir_pfn, &entry, &vaddr_start, &vaddr_end);
    int succeed = load_exec(img, p->page_dir_pfn, &entry, &vaddr_start, &vaddr_end);
    
    if (!succeed) {
        return 0;
    }
    
    // Calculae rounded heap start
    ulong heap_start = vaddr_end;
    if (heap_start % PAGE_SIZE) {
        heap_start /= PAGE_SIZE;
        heap_start++;
        heap_start *= PAGE_SIZE;
    }
    
    // Map the initial page for heap
    ulong heap_paddr = PFN_TO_ADDR(palloc(1));
    succeed = hal->map_user(
        p->page_dir_pfn,
        heap_start, heap_paddr, PAGE_SIZE,
        0, 1, 1, 0
    );
    assert(succeed);
    
    // Set memory layout
    p->memory.entry_point = entry;
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
