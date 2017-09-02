/*
 * System call worker thread - kputs
 */
#include "common/include/kdisp.h"
#include "kernel/include/hal.h"
#include "kernel/include/proc.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"
#include "kernel/include/syscall.h"


#define KPUTS_BUF_SIZE  128

void kputs_worker_thread(ulong param)
{
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    
    struct process *p = disp_info->proc;
    struct thread *worker = disp_info->worker;
    ulong vaddr = disp_info->syscall.param0;
    ulong paddr = 0;
    
//     kprintf("Proc: %s, Vaddr: %p\n", p->name, vaddr);
    
    // Buffer
    char buf[KPUTS_BUF_SIZE + 1];
    memzero(buf, KPUTS_BUF_SIZE + 1);
    int buf_idx = 0;
    
    // Copy to buffer
    do {
        paddr = hal->get_paddr(p->page_dir_pfn, vaddr);
        char *c = (char *)paddr;
        
        if (*c) {
            buf[buf_idx++] = *c;
            vaddr++;
            
            if (buf_idx == KPUTS_BUF_SIZE) {
                kprintf("%s", buf);
                memzero(buf, KPUTS_BUF_SIZE + 1);
                buf_idx = 0;
            }
        } else {
            break;
        }
    } while (1);
    
    // Final print
    if (buf_idx) {
        kprintf("%s", buf);
    }
    
    // Reenable the user thread
    run_thread(disp_info->thread);
    
    // Cleanup
    sfree(disp_info);
    terminate_thread_self(worker);
    
    // Wait for this thread to be terminated
    ksys_unreachable();
}
