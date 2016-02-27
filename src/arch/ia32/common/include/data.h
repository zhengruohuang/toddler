#ifndef __ARCH_IA32_COMMON_INCLUDE_DATA__
#define __ARCH_IA32_COMMON_INCLUDE_DATA__


#ifndef packedstruct
#define packedstruct __attribute__((packed))
#endif

#ifndef asmlinkage
#define asmlinkage __attribute__((regparm(0)))
#endif

#ifndef inthandler
#define inthandler __attribute__((regparm(0)))
#endif

#ifndef no_inline
#define no_inline   __attribute__((noinline))
#endif

#ifndef real_mode
#define real_mode   __attribute__((noinline)) __attribute__((regparm(3))) __attribute__((optimize("-O0")))
#endif

#ifndef no_opt
#define no_opt  __attribute__((optimize("-O0")))
#endif


#ifndef NULL
#define NULL    ((void *)0)
#endif


typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef unsigned char       uchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;
typedef unsigned long long  ulonglong;

typedef unsigned long       size_t;

#endif
