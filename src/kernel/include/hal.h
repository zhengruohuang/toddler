#ifndef __KERNEL_INCLUDE_HAL__
#define __KERNEL_INCLUDE_HAL__


#include "common/include/data.h"
#include "common/include/kexport.h"
#include "common/include/memory.h"
#include "kernel/include/sync.h"


/*
 * HAL exports
 */
extern struct hal_exports *hal;


/*
 * kprintf wrapper
 */
extern spinlock_t kprintf_lock;

#define kprintf(...)                \
do {                                \
    spin_lock_int(&kprintf_lock);   \
    hal->kprintf(__VA_ARGS__);      \
    spin_unlock_int(&kprintf_lock); \
} while (0)


/*
 * Debugging macros
 */
#ifndef disp_src_info
#define disp_src_info(f, b, l) kprintf("[SRC] File: %s, Base: %s, Line: %d\n", f, b, l)
#endif

#ifndef assert
#define assert(exp)                         \
    do {                                    \
        if (exp) {                          \
        } else {                            \
            kprintf("[ASSERT] ");           \
            kprintf(#exp);                  \
            kprintf("\n");                  \
            disp_src_info(__FILE__, __BASE_FILE__, __LINE__);   \
            hal->halt();                    \
        }                                   \
    } while (0)
#endif

#ifndef warn
#define warn(fmt, ...)                  \
    do {                                \
        kprintf("[WARN] ");             \
        kprintf(fmt, ##__VA_ARGS__);    \
        kprintf("\n");                  \
        disp_src_info(__FILE__, __BASE_FILE__, __LINE__);   \
    } while (0)
#endif

#ifndef panic
#define panic(fmt, ...)                 \
    do {                                \
        kprintf("[PANIC] ");            \
        kprintf(fmt, ##__VA_ARGS__);    \
        kprintf("\n");                  \
        disp_src_info(__FILE__, __BASE_FILE__, __LINE__);   \
        hal->halt();                                        \
    } while (0)
#endif


#endif
