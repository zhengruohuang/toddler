#ifndef __KLIBC_INCLUDE_STDARG__
#define __KLIBC_INCLUDE_STDARG__


typedef __builtin_va_list   va_list;
#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_end(ap)          __builtin_va_end(ap)


// typedef void *  va_list;
// 
// 
// #define va_start(ap, last)      ap = (va_list)(void *)(&last); ap += sizeof(last)
// #define va_arg(ap, type)        *((type *)ap); ap += sizeof(type)
// #define va_end(ap)
// #define va_copy(dest, src)      dest = src


#endif
