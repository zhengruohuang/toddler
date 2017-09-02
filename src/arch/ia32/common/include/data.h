#ifndef __ARCH_IA32_COMMON_INCLUDE_DATA__
#define __ARCH_IA32_COMMON_INCLUDE_DATA__


#include "common/include/compiler.h"


#ifdef asmlinkage
#undef asmlinkage
#endif
#define asmlinkage __attribute__((regparm(0)))


#ifndef real_mode
#ifdef __clang__
#define real_mode   __attribute__((noinline)) __attribute__((regparm(3))) __attribute__((optnone))
#else
#define real_mode   __attribute__((noinline)) __attribute__((regparm(3))) __attribute__((optimize("-O0")))
#endif
#endif


#ifndef ARCH_WIDTH
#define ARCH_WIDTH  32
#endif

#ifndef ARCH_LITTLE_ENDIAN
#define ARCH_LITTLE_ENDIAN  1
#endif

#ifndef ARCH_BIG_ENDIAN
#define ARCH_BIG_ENDIAN  0
#endif


#ifndef NULL
#define NULL    ((void *)0)
#endif


#endif
