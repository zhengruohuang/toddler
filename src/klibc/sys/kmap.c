#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"
#include "klibc/include/sys.h"


/*
 * KMap
 */
void *kapi_kmap(enum kmap_region region)
{
    msg_t *s = kapi_msg(KAPI_KMAP);
    msg_t *r = NULL;
    
    msg_param_value(s, (unsigned long)region);
    r = syscall_request();
    
    return (void *)kapi_return_value(r);
}
