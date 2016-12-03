#include "common/include/data.h"
#include "klibc/include/sys.h"


/*
 * Time
 */
u64 get_systime()
{
    unsigned long high = 0;
    unsigned long low = 0;
    u64 time = 0;
    
    syscall_time(&high, &low);
    
    time |= high;
    time <<= sizeof(unsigned long) * 8;
    time |= low;
    
    return time;
}

//  clock_t times(struct tms *buf);
//  int gettimeofday(struct timeval *p, struct timezone *z);
