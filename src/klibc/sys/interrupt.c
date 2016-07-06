#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"


int kapi_interrupt_reg(unsigned long irq, void *handler_entry)
{
    msg_t *s = kapi_msg(KAPI_INTERRUPT_REG);
    
    msg_param_value(s, irq);
    msg_param_value(s, (unsigned long)handler_entry);
    
    syscall_request();
    
    return 1;
}

int kapi_interrupt_unreg(unsigned long irq)
{
    msg_t *s = kapi_msg(KAPI_INTERRUPT_UNREG);
    
    msg_param_value(s, irq);
    
    syscall_request();
    
    return 1;
}
