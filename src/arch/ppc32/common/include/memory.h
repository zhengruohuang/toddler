#ifndef __ARCH_PPC32_COMMON_INCLUDE_MEMORY__
#define __ARCH_PPC32_COMMON_INCLUDE_MEMORY__



#ifndef PAGE_SIZE
#define PAGE_SIZE   4096
#endif

#ifndef PAGE_BITS
#define PAGE_BITS   12
#endif

#ifndef PAGE_ENTRY_COUNT
#define PAGE_ENTRY_COUNT    1024
#endif

#ifndef PAGE_LEVEL
#define PAGE_LEVEL  2
#endif

#ifndef ALIGN_MIN
#define ALIGN_MIN   1
#endif

#ifndef ALIGN_MAX
#define ALIGN_MAX   64
#endif

#ifndef ALIGN_DEFAULT
#define ALIGN_DEFAULT   4
#endif


#ifndef ALIGN_UP
#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))
#endif


#endif
