#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"


unsigned long kapi_get_heap_end()
{
    msg_t *s = kapi_msg(KAPI_HEAP_END_GET);
    msg_t *r = syscall_request();
    
    return kapi_return_value(r);
}

unsigned long kapi_set_heap_end(unsigned long heap_end)
{
    msg_t *s = kapi_msg(KAPI_HEAP_END_SET);
    msg_t *r = NULL;
    
    msg_param_value(s, heap_end);
    r = syscall_request();
    
    return kapi_return_value(r);
}

unsigned long kapi_grow_heap(unsigned long amount)
{
    msg_t *s = kapi_msg(KAPI_HEAP_END_GROW);
    msg_t *r = NULL;
    
    msg_param_value(s, amount);
    r = syscall_request();
    
    return kapi_return_value(r);
}

unsigned long kapi_shrink_heap(unsigned long amount)
{
    msg_t *s = kapi_msg(KAPI_HEAP_END_SHRINK);
    msg_t *r = NULL;
    
    msg_param_value(s, amount);
    r = syscall_request();
    
    return kapi_return_value(r);
}

int kapi_brk(unsigned long heap_end)
{
    unsigned long ret = kapi_set_heap_end(heap_end);
    return ret ? 1 : 0;
}

unsigned long kapi_sbrk(long amount)
{
    if (amount == 0) {
        return kapi_get_heap_end();
    } else if (amount > 0) {
        return kapi_grow_heap((unsigned long)amount);
    } else if (amount < 0) {
        return kapi_shrink_heap((unsigned long)(0 - amount));
    }
    
    return 0;
}
