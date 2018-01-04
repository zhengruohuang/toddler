#ifndef __HAL_INCLUDE_DEBUG__
#define __HAL_INCLUDE_DEBUG__


#include "common/include/data.h"
#include "hal/include/print.h"


/*
 * Arch-specific halt
 */
extern void halt();


/*
 * Herlpers
 */
#ifndef disp_loc
#define disp_loc(file, base, line)          \
    kprintf("[SRC] File: %s, Base: %s, Line: %d\n", file, base, line)
#endif

#ifndef assert
#define assert(exp)                         \
    do {                                    \
        if (exp) {                          \
        } else {                            \
            kprintf("[ASSERT] ");           \
            kprintf(#exp);                  \
            kprintf("\n");                  \
            disp_loc(__FILE__, __BASE_FILE__, __LINE__);   \
            halt();                         \
        }                                   \
    } while (0)
#endif

#ifndef warn
#define warn(fmt, ...)                  \
    do {                                \
        kprintf("[WARN] ");             \
        kprintf(fmt, ##__VA_ARGS__);    \
        kprintf("\n");                  \
        disp_loc(__FILE__, __BASE_FILE__, __LINE__);   \
    } while (0)
#endif

#ifndef panic
#define panic(fmt, ...)                 \
    do {                                \
        kprintf("[PANIC] ");            \
        kprintf(fmt, ##__VA_ARGS__);    \
        kprintf("\n");                  \
        disp_loc(__FILE__, __BASE_FILE__, __LINE__);   \
        halt();                                             \
    } while (0)
#endif


#endif

