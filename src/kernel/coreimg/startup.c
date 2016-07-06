/*
 * Start up user mode processes
 */


#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"
#include "kernel/include/coreimg.h"


static void create_system_process(char *img_name, char *path)
{
    // Create a process
    struct process *p = create_process(0, img_name, path, process_system, 0);
    
    // Load image
    load_image(p, img_name);
    
    // Create the first thread
    struct thread *t = create_thread(p, p->memory.entry_point, 0, -1, 0, 0);
    run_thread(t);
}

void start_user()
{
    kprintf("Starting user processes\n");
    
    create_system_process("tdlrdrv.bin", "coreimg://tdlrdrv.bin");
    create_system_process("tdlrsys.bin", "coreimg://tdlrsys.bin");
    
//     // Create a process
//     struct process *p = create_process(0, "tdlrsys.bin", "coreimg://tdlrsys.bin", process_system, 0);
//     
//     // Load image
//     load_image(p, "tdlrsys.bin");
//     
//     // Create the first thread
//     struct thread *t = create_thread(p, p->memory.entry_point, 0, -1, PAGE_SIZE, PAGE_SIZE);
//     run_thread(t);
    
    kprintf("User processes have been started\n");
    
    //halt();
}
