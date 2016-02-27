#include "common/include/data.h"


ulong get_bits(ulong value, int low, int high)
{
    ulong result = value >> low;
    result -= (value >> high) << high;
    return result;
}

ulong round_up(ulong value)
{
    int i;
    for (i = 0; i < sizeof(ulong) * 8; i++) {
        ulong cur = (ulong)0x1 << i;
        if (cur >= value) {
            return cur;
        }
    }
}

ulong round_down(ulong value)
{
    int i;
    for (i = sizeof(ulong) * 8 - 1; i >= 0; i--) {
        ulong cur = (ulong)0x1 << i;
        if (cur <= value) {
            return cur;
        }
    }
}
