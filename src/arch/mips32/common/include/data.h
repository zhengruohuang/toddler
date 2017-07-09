#ifndef __ARCH_MIPS32_COMMON_INCLUDE_DATA__
#define __ARCH_MIPS32_COMMON_INCLUDE_DATA__


#ifndef packedstruct
#define packedstruct __attribute__((packed))
#endif

#ifndef asmlinkage
#define asmlinkage
#endif

#ifndef no_inline
#define no_inline   __attribute__((noinline))
#endif

#ifndef no_opt
#ifdef __clang__
#define no_opt  __attribute__((optnone))
#else
#define no_opt  __attribute__((optimize("-O0")))
#endif
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

#ifndef AVOID_LIBC_CONFLICT
typedef unsigned long       size_t;
#endif


#endif
