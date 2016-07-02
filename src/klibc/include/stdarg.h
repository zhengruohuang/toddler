#ifndef __KLIBC_INCLUDE_STDARG__
#define __KLIBC_INCLUDE_STDARG__


typedef void *  va_list;


#define va_start(ap, last)      ap = (va_list)(void *)(&last); ap += sizeof(last)
#define va_arg(ap, type)        *((type *)ap); ap += sizeof(type)
#define va_end(ap)
#define va_copy(dest, src)      dest = src


#endif
