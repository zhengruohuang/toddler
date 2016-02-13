#ifndef __ARCH_IA32_COMMON_INCLUDE_DATA__
#define __ARCH_IA32_COMMON_INCLUDE_DATA__


#ifndef packedstruct
#define packedstruct __attribute__((packed))
#endif

#ifndef no_inline
#define no_inline   __attribute__((noinline))
#endif

#ifndef real_mode
#define real_mode   __attribute__((noinline)) __attribute__((regparm(3)))
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


#endif
