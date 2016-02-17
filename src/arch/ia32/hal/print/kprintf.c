#include "common/include/data.h"


#define __FT_UNKNOWN    0
#define __FT_INT        1
#define __FT_HEX        2
#define __FT_BIN        3
#define __FT_INT64      4
#define __FT_HEX64      5
#define __FT_BIN64      6
#define __FT_PTR        7
#define __FT_CHAR       8
#define __FT_STR        9
#define __FT_PERCENT    10


static void print_char(char c)
{
}

static void print_new_line()
{
}

static void print_tab()
{
}

static void print_string(char *s)
{
    char *c = s;
    
    while (*c) {
        print_char(*c++);
    }
}

static void print_hex64(u64 value, int prefix, int upper)
{
    char buf[17];
    int i;
    u64 cur;
    
    for (i = 0; i < 17; i++) {
        buf[i] = 0;
    }
    
    i = 15;
    cur = 0;
    do {
        cur = value & 0xfull;
        if (cur >= (u64)10) {
            buf[i] = (upper ? 'A' : 'a') + (int)cur - 10;
        } else {
            buf[i] = '0' + (int)cur;
        }
        
        value >>= 4;
        i--;
    } while (value);
    
    buf[16] = 0;
    if (prefix) {
        print_string("0x");
    }
    
    for (i = 0; i < 16; i++) {
        if (buf[i] != 0) {
            print_string(buf + i);
            return;
        }
    }
    
    print_string(" __WRONG__HEX64__ ");
}

static void print_int64(u64 value, int has_sign)
{
    char buf[21];
    int i;
    
    for (i = 0; i < 21; i++) {
        buf[i] = 0;
    }
    
    if (value >> 63) {
        print_char('-');
        value = ~value + 1;
    }
    
    i = 19;
    do {
        buf[i] = '0' + (int)(value % (u64)10);
        value /= (u64)10;
        i--;
    } while (value);
    
    buf[20] = 0;
    
    for (i = 0; i < 20; i++) {
        if (buf[i] != 0) {
            print_string(buf + i);
            return;
        }
    }
    
    print_string(" __WRONG__(U)INT__ ");
}

static void print_hex(u32 value, int prefix, int upper)
{
    char buf[9];
    int i;
    unsigned int cur;
    
    for (i = 0; i < 9; i++) {
        buf[i] = 0;
    }
    
    cur = 0;
    i = 7;
    do {
        cur = value & 0xf;
        if (cur >= (u32)10) {
            buf[i] = (upper ? 'A' : 'a') + (int)cur - 10;
        } else {
            buf[i] = '0' + (int)cur;
        }
        
        value >>= 4;
        i--;
    } while (value);
    
    buf[8] = 0;
    if (prefix) {
        print_string("0x");
    }
    
    for (i = 0; i < 8; i++) {
        if (buf[i] != 0) {
            print_string(buf + i);
            return;
        }
    }
    
    print_string(" __WRONG__HEX__ ");
}

static void print_int(u32 value, int has_sign)
{
    char buf[11];
    int i;
    
    for (i = 0; i < 11; i++) {
        buf[i] = 0;
    }
    
    if (has_sign && value >> 31) {
        print_string("-");
        value = ~value + 1;
    }
    
    i = 9;
    do {
        buf[i] = '0' + (int)(value % (u32)10);
        value /= (u32)10;
        i--;
    } while (value);
    
    buf[10] = 0;
    
    for (i = 0; i < 11; i++) {
        if (buf[i]) {
            print_string(buf + i);
            return;
        }
    }
    
    print_string(" __WRONG__(U)INT__ ");
}

static void print_ptr(ulong value)
{
//     if (sizeof(unsigned long) == 64) {
//         _print_hex64(f, (unsigned long long)value);
//     } else {
//         _print_hex(f, (unsigned int)value);
//     }
}

static void print_unknown()
{
    print_string(" __UNKNOWN__ ");
}

static int find_token(char *ft, int *size, int *ft_count, int *prefix, int *upper, int *has_sign)
{
    *ft_count = 0;
    *size = 0;
    *prefix = 0;
    *has_sign = 0;
    
    switch (*ft) {
    case '%':
        *ft_count = 1;
        return __FT_PERCENT;
    case 'c':
    case 'C':
        *ft_count = 1;
        *size = 4;
        return __FT_CHAR;
    case 'd':
    case 'D':
        *ft_count = 1;
        *size = 4;
        *has_sign = 1;
        return __FT_INT;
    case 'u':
    case 'U':
        *ft_count = 1;
        *size = 4;
        return __FT_INT;
    case 'x':
    case 'X':
        *ft_count = 1;
        *size = 4;
        *prefix = 1;
        *upper = (*ft == 'X') ? 1 : 0;
        return __FT_HEX;
    case 'h':
    case 'H':
        *ft_count = 1;
        *size = 4;
        *upper = (*ft == 'H') ? 1 : 0;
        return __FT_HEX;
    case 'b':
    case 'B':
        *ft_count = 1;
        *size = 4;
        return __FT_BIN;
        
    case 'p':
    case 'P':
        *ft_count = 1;
        *size = (sizeof(u64) == sizeof(ulong)) ? 8 : 4;
        *prefix = 1;
        *upper = (*ft == 'P') ? 1 : 0;
        return __FT_PTR;
    case 's':
    case 'S':
        *ft_count = 1;
        *size = (sizeof(u64) == sizeof(ulong)) ? 8 : 4;
        return __FT_STR;
    
    case 'l':
        switch (*(ft + 1)) {
        case 'd':
        case 'D':
            *ft_count = 2;
            *size = 8;
            *has_sign = 1;
            return __FT_INT64;
        case 'u':
        case 'U':
            *ft_count = 2;
            *size = 8;
            return __FT_INT64;
        case 'x':
        case 'X':
            *ft_count = 2;
            *size = 8;
            *prefix = 1;
            *upper = (*(ft + 1) == 'P') ? 1 : 0;
            return __FT_HEX64;
        case 'h':
        case 'H':
            *ft_count = 2;
            *size = 8;
            *upper = (*(ft + 1) == 'P') ? 1 : 0;
            return __FT_HEX64;
        case 'b':
        case 'B':
            *ft_count = 2;
            *size = 8;
            return __FT_BIN64;
            
        case 'l':
            switch (*(ft + 2)) {
            case 'd':
            case 'D':
                *ft_count = 3;
                *size = 8;
                *has_sign = 1;
                return __FT_INT64;
            case 'u':
            case 'U':
                *ft_count = 3;
                *size = 8;
                return __FT_INT64;
            case 'x':
            case 'X':
                *ft_count = 3;
                *size = 8;
                *prefix = 1;
                *upper = (*(ft + 2) == 'P') ? 1 : 0;
                return __FT_HEX64;
            case 'h':
            case 'H':
                *ft_count = 3;
                *size = 8;
                *upper = (*(ft + 2) == 'P') ? 1 : 0;
                return __FT_HEX64;
            case 'b':
            case 'B':
                *ft_count = 3;
                *size = 8;
                return __FT_BIN64;
            default:
                break;
            }
        default:
            break;
        }
        
    default:
        break;
    }
    
    return __FT_UNKNOWN;
}

#define POP_PARAM32()
#define POP_PARAM64()

int asmlinkage kprintf(char *fmt, ...)
{
    char *cur = fmt;
    int ftype = 0, ft_count, param_size, prefix, upper, has_sign;
    
    u32 param32;
    u64 param64;
    u32 va_offset = 12;
    
    while (*cur) {
        switch (*cur) {
        case '\n':
        case '\r':
            print_new_line();
            break;
        case '\t':
            print_tab();
            break;
        case '%':
            // Find out token type
            ftype = find_token(cur + 1, &ft_count, &param_size, &prefix, &upper, &has_sign);
            
            // Pop data from stack
            switch (param_size) {
            case 4:
                param32 = POP_PARAM32();
                break;
            case 8:
                param64 = POP_PARAM64();
                break;
            default:
                break;
            }
            
            // Print the data
            switch (ftype) {
            case __FT_INT:
                
                break;
            case __FT_HEX:
                
                break;
            case __FT_BIN:
                
                break;
            case __FT_INT64:
                
                break;
            case __FT_HEX64:
                
                break;
            case __FT_BIN64:
                
                break;
            case __FT_PTR:
                
                break;
            case __FT_CHAR:
                
                break;
            case __FT_STR:
                
                break;
            case __FT_PERCENT:
                print_char('%');
                break;
            case __FT_UNKNOWN:
            default:
                print_unknown();
                break;
            }
            
            /* Pop a param from stack */
//             __asm__ __volatile__
//             (
//                 "movl   (%%ebp, %%ebx, 1), %%eax;"
//                 : "=a" (param)
//                 : "b" (ebp_disp)
//             );
//             va_offset += sizeof(u32);
            
            cur += ft_count;
            break;
        default:
            print_char(*cur);
            break;
        }
        
        cur++;
    }
    
}
