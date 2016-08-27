#ifndef __KLIBC_INCLUDE_ASSERT__
#define __KLIBC_INCLUDE_ASSERT__


#include "klibc/include/stdio.h"


#ifdef assert
#undef assert
#endif

#define assert(exp) do {                \
    if (!(exp)) {                       \
        kprintf("[ASSERT] Failed: ");   \
        kprintf(#exp);                  \
        kprintf("\n");                  \
        kprintf("[SRC] File: %s, Base: %s, Line: %d\n", __FILE__, __BASE_FILE__, __LINE__); \
    }                                   \
} while (0)


#endif
