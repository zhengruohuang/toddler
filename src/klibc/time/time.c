#include "common/include/data.h"
#include "klibc/include/sys.h"
#include "klibc/include/time.h"


/*
 * Time
 */
time_t time()
{
    unsigned long high = 0;
    unsigned long low = 0;
    time_t time = 0;
    
    syscall_time(&high, &low);
    
    time |= high;
    time <<= sizeof(unsigned long) * 8;
    time |= low;
    
    return time;
}

//  clock_t times(struct tms *buf);
//  int gettimeofday(struct timeval *p, struct timezone *z);
