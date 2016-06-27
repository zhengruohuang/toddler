/*
 * System call workers - IPC
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/lib.h"
#include "kernel/include/ds.h"


static int msg_salloc_id;
static int msg_node_salloc_id;
static int msg_handler_salloc_id;

static hashtable_t kapi_servers;


void init_ipc()
{
    msg_salloc_id = salloc_create(sizeof(msg_t), 0, 0, NULL, NULL);
    msg_node_salloc_id = salloc_create(sizeof(struct msg_node), 0, 0, NULL, NULL);
    msg_handler_salloc_id = salloc_create(sizeof(struct msg_handler), 0, 0, NULL, NULL);
    
    hashtable_create(&kapi_servers, 0, NULL);
    
    kprintf("\tIPC node salloc IDs, Message: %d, Node: %d, Handler: %d\n",
            msg_salloc_id, msg_node_salloc_id, msg_handler_salloc_id
    );
}


void reg_msg_handler_worker_thread(ulong param)
{
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    struct thread *worker = disp_info->syscall.worker;
    
    struct process *p = disp_info->proc;
    ulong msg_num = disp_info->syscall.param0;
    ulong thread_entry = disp_info->syscall.param1;
    
    // Register the msg handler
    hashtable_insert(&p->msg_handlers, msg_num, (void *)thread_entry);
    
    // Reenable the user thread
    run_thread(disp_info->thread);
    
    // Cleanup
    free(disp_info);
    terminate_thread(worker);
    
    // Wait for this thread to be terminated
    do {
        hal->sleep();
    } while (1);
    
    // Should never reach here
    kprintf("kputs.c: Should never reach here!\n");
    do {
        hal->sleep();
    } while (1);
}

void unreg_msg_handler_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong msg_num = disp_info->syscall.param0;
    
    // Register the msg handler
    hashtable_remove(&p->msg_handlers, msg_num);
}


void reg_kapi_server_worker_thread(ulong param)
{
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    struct thread *worker = disp_info->syscall.worker;
    
    struct process *p = disp_info->proc;
    ulong kapi_num = disp_info->syscall.param0;
    
    // Unregister the KAPI server
    hashtable_insert(&kapi_servers, kapi_num, (void *)p);
    
    kprintf("KAPI num: %p registered\n", kapi_num);
    
    // Reenable the user thread
    run_thread(disp_info->thread);
    
    // Cleanup
    free(disp_info);
    terminate_thread(worker);
    
    // Wait for this thread to be terminated
    do {
        hal->sleep();
    } while (1);
    
    // Should never reach here
    kprintf("kputs.c: Should never reach here!\n");
    do {
        hal->sleep();
    } while (1);
}

void unreg_kapi_server_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    //struct process *p = disp_info->proc;
    ulong kapi_num = disp_info->syscall.param0;
    
    // Unregister the KAPI server
    hashtable_remove(&kapi_servers, kapi_num);
}


static struct process *obtain_kapi_server(int kapi_num)
{
    struct process *p = (struct process *)hashtable_obtain(&kapi_servers, kapi_num);
    if (p) {
        hashtable_release(&kapi_servers, kapi_num, p);
    }
    
    return p;
}

static struct process *get_process_by_mailbox_id(struct process *src_p,
    ulong mailbox_id, ulong opcode, ulong func)
{
    struct process *p = NULL;
    
    kprintf("mailbox_id: %p, opcode: %p, func: %p\n", mailbox_id, opcode, func);
    
    if (opcode != IPC_OPCODE_NONE) {
        switch (mailbox_id) {
        case IPC_MAILBOX_NONE:
            break;
        case IPC_MAILBOX_KERNEL:
            kprintf("mbox kernel\n");
            if (opcode == IPC_OPCODE_KAPI) {
                p = obtain_kapi_server(func);
                kprintf("user proc obtained\n");
            }
            if (!p) {
                kprintf("to obtain kernel proc\n");
                p = kernel_proc;
            }
            break;
        case IPC_MAILBOX_THIS_PROCESS:
            p = src_p;
            break;
        default:
            if (mailbox_id) {
                p = (struct process *)mailbox_id;
            }
        }
    }
    
    return p;
}

static struct thread *get_thread_by_mailbox_id(ulong mailbox_id)
{
    return (struct thread *)mailbox_id;
}

static struct msg_node *duplicate_msg(
    msg_t *s, int sender_blocked,
    struct process *src_p, struct thread *src_t,
    struct process *dest_p, struct thread *dest_t)
{
    // Setup msg node
    struct msg_node *n = (struct msg_node *)salloc(msg_node_salloc_id);
    
    n->src.proc = src_p;
    n->src.thread = src_t;
    n->src.mailbox_id = src_p->mailbox_id;
    
    n->dest.proc = dest_p;
    n->dest.thread = dest_t;
    n->dest.mailbox_id = s->mailbox_id;
    
    n->sender_blocked = sender_blocked;
    
    // Create a new copy of the msg content
    n->msg = (msg_t *)salloc(msg_salloc_id);
    memcpy((void *)s, (void *)n->msg, s->msg_size);
    
    return n;
}

static void free_msg(struct msg_node *n)
{
    assert(n);
    assert(n->msg);
    
    sfree((void *)n->msg);
    sfree(n);
}

static void copy_msg_to_recv(struct thread *t)
{
    msg_t *src = t->cur_msg->msg;
    msg_t *dest = (msg_t *)t->memory.msg_recv_paddr;
    
    memcpy((void *)src, (void *)dest, src->msg_size);
}

static void transfer_msg(msg_t *s, int sender_blocked, struct process *src_p, struct thread *src_t)
{
    kprintf("msg info, size: %d, param: %d, msg start paddr: %p, block start vaddr: %p, kernel: %d\n",
            s->msg_size, s->param_count,
            src_t->memory.msg_send_paddr, src_t->memory.thread_block_base,
            (src_p->type == process_kernel) ? 1 : 0
           );
    
    // Get dest info
    struct process *dest_p = get_process_by_mailbox_id(src_p, s->mailbox_id, s->opcode, s->func_num);
    if (!dest_p) {
        return;
    }
    
    kprintf("Transferring msg!\n");
    __asm__ __volatile__
    (
        "xchgw %%bx, %%bx;"
        :
        :
    );
    
    // Setup msg node
    struct msg_node *n = duplicate_msg(s, sender_blocked, src_p, src_t, dest_p, NULL);
    
    if (hashtable_contains(&src_p->msg_handlers, s->func_num)) {
        // Create a new thread to handle the msg
        void *entry_point = hashtable_obtain(&src_p->msg_handlers, s->func_num);
        struct thread *t = create_thread(dest_p, (ulong)entry_point, 0, -1, PAGE_SIZE, PAGE_SIZE);
        
        // Attach the msg to the thread
        t->cur_msg = n;
        
        // Copy the msg content to thread's recv window
        copy_msg_to_recv(t);
        
        // Run the thread
        run_thread(t);
        
        // Release the hashtable node
        hashtable_release(&src_p->msg_handlers, s->func_num, entry_point);
    } else {
        // Push the node into the dest's msg queue
        list_push_back(&dest_p->msgs, n);
    }
}

void send_worker(struct kernel_dispatch_info *disp_info)
{
    // Get src info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    msg_t *s = (msg_t *)src_t->memory.msg_send_paddr;
    assert(s);
    
    // Transfer msg
    transfer_msg(s, 0, src_p, src_t);
}

void reply_worker(struct kernel_dispatch_info *disp_info)
{
    // Get src info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    msg_t *s = (msg_t *)src_t->memory.msg_send_paddr;
    assert(s);
    
    // Obtain the msg that is being replied to
    struct msg_node *n = src_t->cur_msg;
    assert(n);
    assert(n->sender_blocked);
    
    // Get dest info
    struct thread *dest_t = get_thread_by_mailbox_id(s->mailbox_id); //n->dest.thread;
    struct process *dest_p = dest_t->proc; //n->dest.proc;
    
    // Setup msg node
    struct msg_node *dest_n = duplicate_msg(n->msg, 0, src_p, src_t, dest_p, dest_t);

    // Reply to the thread
    if (dest_t->cur_msg) {
        free_msg(dest_t->cur_msg);
    }
    dest_t->cur_msg = dest_n;
    
    // Copy the msg to the recv window
    copy_msg_to_recv(dest_t);
    
    // Wake up the thread
    run_thread(dest_t);
}

void recv_worker_thread(ulong param)
{
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    struct thread *worker = disp_info->syscall.worker;
    
    // Get recv info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    
    // Pop a msg
    struct msg_node *s = NULL;
    do {
        s = list_pop_front(&src_p->msgs);
        if (s) {
            break;
        }
        
        hal->sleep();
    } while (1);
    
    // Clean the previous msg
    if (src_t->cur_msg) {
        free_msg(src_t->cur_msg);
    }
    
    // Attach the msg to current thread
    src_t->cur_msg = s;
    
    // Copy the msg content to thread's recv window
    copy_msg_to_recv(src_t);
    
    // Wakeup the thread
    run_thread(src_t);
    
    // Cleanup
    free(disp_info);
    terminate_thread(worker);
    
    // Should never reach here
    do {
        kprintf("Should never reach here!\n");
        hal->sleep();
    } while (1);
}

void request_worker_thread(ulong param)
{
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    struct thread *worker = disp_info->syscall.worker;
    
    // Get src info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    msg_t *s = (msg_t *)src_t->memory.msg_send_paddr;
    assert(s);
    
    kprintf("To transfer msg!\n");
    
    // Transfer msg
    transfer_msg(s, 1, src_p, src_t);
    
    // Cleanup
    free(disp_info);
    terminate_thread(worker);
    
    // Should never reach here
    do {
        kprintf("Should never reach here!\n");
        hal->sleep();
    } while (1);
}

void respond_worker_thread(ulong param)
{
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    struct thread *worker = disp_info->syscall.worker;
    struct thread *src_t = disp_info->thread;
    
    // Do a reply
    reply_worker(disp_info);
    
    // Terminate the src thread
    terminate_thread(src_t);
    
    // Cleanup
    free(disp_info);
    terminate_thread(worker);
    
    // Should never reach here
    do {
        kprintf("Should never reach here!\n");
        hal->sleep();
    } while (1);
}
