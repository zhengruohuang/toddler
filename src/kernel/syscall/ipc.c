/*
 * System call workers - IPC
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/ds.h"


static int msg_salloc_id;
static int msg_node_salloc_id;
static int msg_handler_salloc_id;


void init_ipc()
{
    msg_salloc_id = salloc_create(sizeof(msg_t), 0, 0, NULL, NULL);
    msg_node_salloc_id = salloc_create(sizeof(struct msg_node), 0, 0, NULL, NULL);
    msg_handler_salloc_id = salloc_create(sizeof(struct msg_handler), 0, 0, NULL, NULL);
    
    kprintf("\tIPC node salloc IDs, Message: %d, Node: %d, Handler: %d\n",
            msg_salloc_id, msg_node_salloc_id, msg_handler_salloc_id
    );
}


void reg_handler_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong msg_num = disp_info->syscall.param0;
    //ulong thread_entry = disp_info->syscall.param1;
    
    // Register the msg handler
    hashtable_insert(&p->msg_handlers, msg_num, NULL);
}

void rel_handler_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong msg_num = disp_info->syscall.param0;
    
    // Register the msg handler
    hashtable_remove(&p->msg_handlers, msg_num);
}

static struct process *get_process_by_mailbox_id(ulong mailbox_id)
{
    return NULL;
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
    n->dest.mailbox_id = s->dest_mailbox_id;
    
    n->sender_blocked = sender_blocked;
    
    // Create a new copy of the msg content
    n->msg = (msg_t *)salloc(msg_salloc_id);
    
    return n;
}

static void free_msg(struct msg_node *n)
{
}

static void transfer_msg(msg_t *s, int sender_blocked, struct process *src_p, struct thread *src_t)
{
    // Get dest info
    struct process *dest_p = get_process_by_mailbox_id(s->dest_mailbox_id);
    
    // Setup msg node
    struct msg_node *n = duplicate_msg(s, sender_blocked, src_p, src_t, dest_p, NULL);
    
    if (hashtable_contains(&src_p->msg_handlers, s->msg_num)) {
        // Create a new thread to handle the msg
        
        // Attach the msg to the thread
        
        // Copy the msg content to thread's recv window
        
        // Run the thread
    } else {
        // Push the node into the dest's msg queue
        list_push_back(&dest_p->msgs, n);
    }
}

void send_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    msg_t *s = (msg_t *)disp_info->syscall.param0;
    assert(s);
    
    // Get src info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    
    // Transfer msg
    transfer_msg(s, 0, src_p, src_t);
}

void reply_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    msg_t *s = (msg_t *)disp_info->syscall.param0;
    assert(s);
    
    // Get src info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    
    // Obtain the msg that is being replied to
    struct msg_node *n = src_t->cur_msg;
    assert(n);
    assert(n->sender_blocked);
    
    // Get dest info
    struct process *dest_p = n->dest.proc;
    struct thread *dest_t = n->dest.thread;
    
    // Setup msg node
    struct msg_node *dest_n = duplicate_msg(n->msg, 0, src_p, src_t, dest_p, dest_t);

    // Reply to the thread
    if (dest_t->cur_msg) {
        free_msg(dest_t->cur_msg);
    }
    dest_t->cur_msg = dest_n;
    
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
    msg_t *s = (msg_t *)disp_info->syscall.param0;
    assert(s);
    
    // Get src info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    
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
