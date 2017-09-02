#include "common/include/data.h"


/*
 * First non-zero bit from the left
 */
int fnzb32(u32 arg)
{
    int n = 0;

    if (arg >> 16) {
        arg >>= 16;
        n += 16;
    }

    if (arg >> 8) {
        arg >>= 8;
        n += 8;
    }

    if (arg >> 4) {
        arg >>= 4;
        n += 4;
    }

    if (arg >> 2) {
        arg >>= 2;
        n += 2;
    }

    if (arg >> 1) {
        arg >>= 1;
        n += 1;
    }

    return n;
}

int fnzb64(u64 arg)
{
    int n = 0;

    if (arg >> 32) {
        arg >>= 32;
        n += 32;
    }

    return n + fnzb32((u32)arg);
}
