#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"


static unsigned long cur_msg_num = USER_MSG_NUM_BASE;


unsigned long alloc_msg_num()
{
    return cur_msg_num++;
}
