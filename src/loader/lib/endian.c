#include "common/include/data.h"


u64 swap_endian64(u64 val)
{
    u64 rrr = val & 0xffull;
    u64 rrl = (val >> 8) & 0xffull;
    
    u64 rlr = (val >> 16) & 0xffull;
    u64 rll = (val >> 24) & 0xffull;
    
    u64 lrr = (val >> 32) & 0xffull;
    u64 lrl = (val >> 40) & 0xffull;
    
    u64 llr = (val >> 48) & 0xffull;
    u64 lll = (val >> 56) & 0xffull;
    
    u64 swap = (rrr << 56) | (rrl << 48) |
        (rlr << 40) | (rll << 32) |
        (lrr << 24) | (lrl << 16) |
        (llr << 8) | lll;
    
    return swap;
}

u32 swap_endian32(u32 val)
{
    u32 rr = val & 0xff;
    u32 rl = (val >> 8) & 0xff;
    u32 lr = (val >> 16) & 0xff;
    u32 ll = (val >> 24) & 0xff;
    
    u32 swap = (rr << 24) | (rl << 16) | (lr << 8) | ll;
    
    return swap;
}

u16 swap_endian16(u16 val)
{
    u16 r = val & 0xff;
    u16 l = (val >> 8) & 0xff;
    
    u16 swap = (r << 8) | l;
    
    return swap;
}

ulong swap_endian(ulong val)
{
#if (ARCH_WIDTH == 64)
    return (ulong)swap_endian64((u64)val);
#elif (ARCH_WIDTH == 32)
    return (ulong)swap_endian32((u32)val);
#elif (ARCH_WIDTH == 16)
    return (ulong)swap_endian16((u16)val);
#else
    #error Unsupport ARCH_WIDTH
#endif
}
