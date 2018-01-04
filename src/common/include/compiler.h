#ifndef __COMMON_INCLUDE_COMPILER__
#define __COMMON_INCLUDE_COMPILER__


#ifndef asmlinkage
#define asmlinkage
#endif


#ifndef packedstruct
#define packedstruct __attribute__((packed))
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

#ifndef weak_func
#define weak_func __attribute__((weak))
#endif

#ifndef weak_alias
#define weak_alias(f) __attribute__((weak, alias(#f)))
#endif

#ifndef entry_func
#define entry_func  __attribute__((section("entry")))
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
