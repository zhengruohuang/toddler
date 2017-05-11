/*
 * Process monitor
 */


#include "common/include/data.h"
#include "common/include/errno.h"
#include "common/include/proc.h"
#include "kernel/include/hal.h"
#include "kernel/include/sync.h"
#include "kernel/include/kapi.h"
#include "kernel/include/proc.h"


struct monitor_dispatch_info {
    unsigned long proc_id;
    unsigned long func_num;
    unsigned long opcode;
};


static struct monitor_dispatch_info dispatch_info[pm_type_count];


static msg_t *create_dispatch_msg(enum proc_monitor_type type)
{
    msg_t *msg = create_request_msg();
    
    msg->mailbox_id = dispatch_info[type].proc_id;
    msg->opcode = dispatch_info[type].opcode;
    msg->func_num = dispatch_info[type].func_num;
    
    return msg;
}

int check_process_create_before(unsigned long parent_proc_id)
{
    if (!dispatch_info[pm_create_before].proc_id) {
        return EOK;
    }
    
    msg_t *s = create_dispatch_msg(pm_create_before);
    set_msg_param_value(s, parent_proc_id);
    
    msg_t *r = ksys_request();
    
    int result = (int)r->params[r->param_count - 1].value;
    return result;
}

int check_process_create_after(unsigned long parent_proc_id, unsigned long proc_id)
{
    if (!dispatch_info[pm_create_after].proc_id) {
        return EOK;
    }
    
    msg_t *s = create_dispatch_msg(pm_create_after);
    set_msg_param_value(s, parent_proc_id);
    set_msg_param_value(s, proc_id);
    
    msg_t *r = ksys_request();
    
    int result = (int)r->params[r->param_count - 1].value;
    return result;
}

int check_process_terminate_before(unsigned long proc_id)
{
    if (!dispatch_info[pm_terminate_before].proc_id) {
        return EOK;
    }
    
    msg_t *s = create_dispatch_msg(pm_terminate_before);
    set_msg_param_value(s, proc_id);
    
    msg_t *r = ksys_request();
    
    int result = (int)r->params[r->param_count - 1].value;
    return result;
}

int check_process_terminate_after(unsigned long proc_id)
{
    if (!dispatch_info[pm_terminate_after].proc_id) {
        return EOK;
    }
    
    msg_t *s = create_dispatch_msg(pm_terminate_after);
    set_msg_param_value(s, proc_id);
    
    msg_t *r = ksys_request();
    
    int result = (int)r->params[r->param_count - 1].value;
    return result;
}


int register_process_monitor(enum proc_monitor_type type, unsigned long proc_id, unsigned long func_num, unsigned long opcode)
{
    //kprintf("Reg proc monitor, type: %d, proc id: %x, func_num: %x, opcode: %x\n", type, proc_id, func_num, opcode);
    
    assert(dispatch_info[type].proc_id == 0);
    
    dispatch_info[type].proc_id = proc_id;
    dispatch_info[type].func_num = func_num;
    dispatch_info[type].opcode = opcode;
    
    return EOK;
}


void init_process_monitor()
{
    int i;
    for (i = 0; i < pm_type_count; i++) {
        dispatch_info[i].proc_id = 0;
        dispatch_info[i].func_num = 0;
        dispatch_info[i].opcode = 0;
    }
}
