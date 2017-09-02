
void get_system_time(unsigned long *high, unsigned long *low)
{
//     kprintf("\tTimestamp: %p-%p\n",
//             (unsigned long)(cur_timestamp >> sizeof(unsigned long) * 8),
//             (unsigned long)cur_timestamp
//     );
    
    if (high) {
        *high = 0;
    }
    
    if (low) {
        *low = 0;
    }
}
