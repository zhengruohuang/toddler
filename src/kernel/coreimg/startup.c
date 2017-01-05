/*
 * Start up user mode processes
 */


#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"
#include "kernel/include/coreimg.h"


struct startup_record {
    char *name;
    char *url;
    enum process_type type;
    volatile ulong proc_id;
    volatile int started;
};


static struct startup_record records[] = {
    { "tdlrsys.bin", "coreimg://tdlrsys.bin", process_system, 0, 0 },
    { "tdlrdrv.bin", "coreimg://tdlrdrv.bin", process_driver, 0, 0 },
    { "tdlrshell.bin", "coreimg://tdlrshell.bin", process_user, 0, 0 },
};

static struct thread *worker = NULL;


static unsigned long create_startup_process(char *name, char *url, enum process_type type)
{
    kprintf("To create: %s @ %s, type: %d\n", name, url, type);
    
    // Create a process
    struct process *p = create_process(0, name, url, type, 0);
    
    // Load image
    load_image(p, name);
    
    // Create the first thread
    struct thread *t = create_thread(p, p->memory.entry_point, 0, -1, 0, 0);
    run_thread(t);
    
    return p->proc_id;
}

static void create_startup_worker(ulong param)
{
    int i;
    
    for (i = 0; i < sizeof(records) / sizeof(struct startup_record); i++) {
        // Create the process
        records[i].proc_id = create_startup_process(records[i].name, records[i].url, records[i].type);
        atomic_membar();
        
        // Wait until the process is initialized
        do {
            hal->yield();
            atomic_membar();
        } while (!records[i].started);
    }
    
    kprintf("All startup processes have been started!\n");
    
    // Done
    terminate_thread_self(worker);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}

void startup_process_started(ulong proc_id)
{
    int i;
    
    if (!proc_id) {
        return;
    }
    
    for (i = 0; i < sizeof(records) / sizeof(struct startup_record); i++) {
        if (records[i].proc_id == proc_id) {
            records[i].started = 1;
            atomic_membar();
            
            return;
        }
    }
}


void start_user()
{
    kprintf("Starting startup processes\n");
    
    worker = create_thread(kernel_proc, (ulong)&create_startup_worker, 0, -1, 0, 0);
    run_thread(worker);
    
    kprintf("\tStarter thread created, thread ID: %p\n", worker->thread_id);
    
    //create_startup_process("tdlrsys.bin", "coreimg://tdlrsys.bin", process_system);
    //create_startup_process("tdlrdrv.bin", "coreimg://tdlrdrv.bin", process_driver);
    
    //kprintf("User processes have been started\n");
    
    //halt();
}
