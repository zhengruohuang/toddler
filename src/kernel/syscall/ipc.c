/*
 * System call workers - IPC
 */
#include "common/include/task.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/lib.h"
#include "kernel/include/ds.h"
#include "kernel/include/kapi.h"


static int msg_salloc_id;
static int msg_node_salloc_id;
static int msg_handler_salloc_id;
static int kernel_msg_handler_arg_salloc_id;


void init_ipc()
{
    msg_salloc_id = salloc_create(sizeof(msg_t), 0, 0, NULL, NULL);
    msg_node_salloc_id = salloc_create(sizeof(struct msg_node), 0, 0, NULL, NULL);
    msg_handler_salloc_id = salloc_create(sizeof(struct msg_handler), 0, 0, NULL, NULL);
    kernel_msg_handler_arg_salloc_id = salloc_create(sizeof(struct kernel_msg_handler_arg), 0, 0, NULL, NULL);
    
    kprintf("\tIPC node salloc IDs, Message: %d, Node: %d, Handler: %d, Kernel Msg Handler Arg: %d\n",
            msg_salloc_id, msg_node_salloc_id, msg_handler_salloc_id, kernel_msg_handler_arg_salloc_id
    );
}


void reg_msg_handler_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong msg_num = disp_info->syscall.param0;
    ulong thread_entry = disp_info->syscall.param1;
    
    // Register the msg handler
    hashtable_insert(&p->msg_handlers, msg_num, (void *)thread_entry);
}

void unreg_msg_handler_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong msg_num = disp_info->syscall.param0;
    
    // Register the msg handler
    hashtable_remove(&p->msg_handlers, msg_num);
}


void reg_kapi_server_worker(struct kernel_dispatch_info *disp_info)
{
    // Get the params
    struct process *p = disp_info->proc;
    ulong kapi_num = disp_info->syscall.param0;
    
    // Unregister the KAPI server
    hashtable_insert(&kapi_servers, kapi_num, (void *)p);
    
    //kprintf("KAPI num: %p registered\n", kapi_num);
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
    
//     kprintf("mailbox_id: %p, opcode: %p, func: %p\n", mailbox_id, opcode, func);
    
    if (opcode != IPC_OPCODE_NONE) {
        switch (mailbox_id) {
        case IPC_MAILBOX_NONE:
            break;
        case IPC_MAILBOX_KERNEL:
            //kprintf("mbox kernel\n");
            if (opcode == IPC_OPCODE_KAPI) {
                p = obtain_kapi_server(func);
                //kprintf("user proc obtained\n");
            }
            if (!p) {
                //kprintf("to obtain kernel proc\n");
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
    memcpy((void *)n->msg, (void *)s, s->msg_size);
    n->msg->mailbox_id = (ulong)src_t;
    
    return n;
}

static void sfree_msg(struct msg_node *n)
{
    assert(n);
    assert(n->msg);
    
    sfree((void *)n->msg);
    sfree(n);
}

static void copy_msg_to_recv(msg_t *src, struct thread *t)
{
    //msg_t *src = t->cur_msg->msg;
    msg_t *dest = (msg_t *)t->memory.msg_recv_paddr;
    
//     kprintf("dest: %p, src: %p\n", dest, src);
    
    memcpy((void *)dest, (void *)src, src->msg_size);
}

static void transfer_msg(msg_t *s, int sender_blocked, struct process *src_p, struct thread *src_t)
{
//     kprintf("msg info, size: %d, param: %d, msg start paddr: %p, block start vaddr: %p, kernel: %d\n",
//             s->msg_size, s->param_count,
//             src_t->memory.msg_send_paddr, src_t->memory.block_base,
//             (src_p->type == process_kernel) ? 1 : 0
//            );
    
    // Get dest info
    struct process *dest_p = get_process_by_mailbox_id(src_p, s->mailbox_id, s->opcode, s->func_num);
    if (!dest_p) {
        return;
    }
    
//     kprintf("Transferring msg!\n");
//     __asm__ __volatile__
//     (
//         "xchgw %%bx, %%bx;"
//         :
//         :
//     );
    
//     kprintf("Msg duplicated!\n");
    
    // Transfer the msg
    if (hashtable_contains(&dest_p->msg_handlers, s->func_num)) {
//         kprintf("To create thread!\n");
        
        // Create a new thread to handle the msg
        void *entry_point = hashtable_obtain(&dest_p->msg_handlers, s->func_num);
//         kprintf("entry point: %p\n", entry_point);
        struct thread *t = create_thread(dest_p, (ulong)entry_point, 0, -1, PAGE_SIZE, PAGE_SIZE);
        hashtable_release(&dest_p->msg_handlers, s->func_num, entry_point);
        
//         kprintf("To create thread!\n");
        
        // Prepare the arguments
        if (dest_p->type == process_kernel) {
            struct kernel_msg_handler_arg *arg = (struct kernel_msg_handler_arg *)salloc(kernel_msg_handler_arg_salloc_id);
            arg->handler_thread = t;
            arg->sender_thread = src_t;
            arg->msg = (msg_t *)(void *)(t->memory.block_base + t->memory.msg_recv_offset);
            set_thread_arg(t, (ulong)arg);
        } else {
            set_thread_arg(t, t->memory.block_base + t->memory.msg_recv_offset);
        }
        
//         kprintf("Thread created!\n");
        
        // Attach the msg to the thread
        //t->cur_msg = n;
        
        // Copy the msg content to thread's recv window
        s->mailbox_id = (ulong)src_t;
        copy_msg_to_recv(s, t);
        
        // Run the thread
        run_thread(t);
    } else {
        // Push the node into the dest's msg queue
        kprintf("To push to msg queue!\n");
        
        // Setup msg node
        struct msg_node *n = duplicate_msg(s, sender_blocked, src_p, src_t, dest_p, NULL);
        list_push_back(&dest_p->msgs, n);
        
        kprintf("Pushed to msg queue!\n");
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
//     kprintf("To reply!\n");
    
    // Get src info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    msg_t *s = (msg_t *)src_t->memory.msg_send_paddr;
    assert(s);
    
    ulong vaddr = src_t->memory.block_base + src_t->memory.msg_send_offset;
//     kprintf("Msg vaddr: %x, paddr: %x, mapped: %x\n", vaddr, src_t->memory.msg_send_paddr, hal->get_paddr(src_p->page_dir_pfn, vaddr));
//     kprintf("Src: %s, msg @ %x, func num: %x, opcode: %x, size: %x\n", src_p->name, s, s->func_num, s->opcode, s->msg_size);
    
    // Get dest info
    struct thread *dest_t = get_thread_by_mailbox_id(s->mailbox_id); //n->dest.thread;
    struct process *dest_p = dest_t->proc; //n->dest.proc;
    
//     kprintf("Dest @ %x\n", dest_t);
    
//     kprintf("dest t: %p, dest p: %p\n", dest_t, dest_p);
    
//     // Setup msg node
//     struct msg_node *dest_n = duplicate_msg(s, 0, src_p, src_t, dest_p, dest_t);
// 
//     // Reply to the thread
//     if (dest_t->cur_msg) {
//         sfree_msg(dest_t->cur_msg);
//     }
//     dest_t->cur_msg = dest_n;
    
    // Copy the msg to the recv window
    copy_msg_to_recv(s, dest_t);
    
    // Wake up the thread
//     kprintf("To wake up receiver thread @ %x!\n", dest_t);
    run_thread(dest_t);
}

void recv_worker_thread(ulong param)
{
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    struct thread *worker = disp_info->worker;
    
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
        
        hal->yield();
    } while (1);
    
//     // Clean the previous msg
//     if (src_t->cur_msg) {
//         sfree_msg(src_t->cur_msg);
//     }
//     
//     // Attach the msg to current thread
//     src_t->cur_msg = s;
    
    // Copy the msg content to thread's recv window
    copy_msg_to_recv(s->msg, src_t);
    sfree_msg(s);
    
    // Wakeup the thread
    run_thread(src_t);
    
    // Cleanup
    sfree(disp_info);
    terminate_thread_self(worker);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}

void request_worker_thread(ulong param)
{
//     kprintf("Request worker thread!\n");
    
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    struct thread *worker = disp_info->worker;
    
    // Get src info
    struct process *src_p = disp_info->proc;
    struct thread *src_t = disp_info->thread;
    msg_t *s = (msg_t *)src_t->memory.msg_send_paddr;
    assert(s);
    
//     kprintf("To transfer msg!\n");
    
    // Transfer msg
    transfer_msg(s, 1, src_p, src_t);
    
    // Cleanup
    sfree(disp_info);
    terminate_thread_self(worker);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}

void respond_worker_thread(ulong param)
{
    // Get the params
    struct kernel_dispatch_info *disp_info = (struct kernel_dispatch_info *)param;
    struct thread *worker = disp_info->worker;
    struct thread *src_t = disp_info->thread;
    
    // Do a reply
    reply_worker(disp_info);
    
//     kprintf("To terminate src thread!\n");
    
    // Terminate the src thread
    terminate_thread(src_t);
    
//     kprintf("To terminate worker thread!\n");
    
    // Cleanup
    sfree(disp_info);
    terminate_thread_self(worker);
    
//     kprintf("Worker thread terminated!\n");
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}
