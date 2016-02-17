#include "common/include/data.h"
#include "hal/include/print.h"


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
    draw_char(c);
}

static void print_string(char *s)
{
    char *c = s;
    
    while (*c) {
        print_char(*c++);
    }
}

static void print_bin64(u64 value)
{
    int i;
    
    for (i = 63; i >= 0; i++) {
        print_char((value & (0x1ull << i)) ? '1' : '0');
    }
}

static void print_bin(u32 value)
{
    int i;
    
    for (i = 31; i >= 0; i++) {
        print_char((value & (0x1 << i)) ? '1' : '0');
    }
}

static void print_hex64(u64 value, int prefix, int upper)
{
    int i;
    u64 cur = 0;
    int started = 0;
    
    if (prefix) {
        print_string("0x");
    }
    
    for (i = 60; i >= 0; i -= 4) {
        cur = (value >> i) & 0xfull;
        
        if (cur >= (u64)10) {
            print_char((upper ? 'A' : 'a') + (int)cur - 10);
            started = 1;
        } else if (cur || started) {
            print_char('0' + (int)cur);
            started = 1;
        }
    }
    
    if (!started) {
        print_char('0');
    }
}

static void print_hex(u32 value, int prefix, int upper)
{
    int i;
    u32 cur = 0;
    int started = 0;
    
    if (prefix) {
        print_string("0x");
    }
    
    for (i = 28; i >= 0; i -= 4) {
        cur = (value >> i) & 0xf;
        
        if (cur >= (u32)10) {
            print_char((upper ? 'A' : 'a') + (int)cur - 10);
            started = 1;
        } else if (cur || started) {
            print_char('0' + (int)cur);
            started = 1;
        }
    }
    
    if (!started) {
        print_char('0');
    }
}

static void print_int64(u64 value, int has_sign)
{
    print_string("__UNSUPPORT_INT64__");
}

static void print_int(u32 value, int has_sign)
{
    int i;
    u32 div = 1000000000;
    u32 cur = 0;
    int started = 0;
    
    if (has_sign && value >> 31) {
        print_string("-");
        value = ~value + 1;
    }
    
    while (div) {
        cur = value / div;
        
        if (cur || started) {
            print_char('0' + (int)cur);
            started = 1;
        }
        
        value %= div;
        div /= 10;
    }
    
    if (!started) {
        print_char('0');
    }
}

static void print_unknown()
{
    print_string(" __UNKNOWN__ ");
}

static int find_long_token(char token, int *size, int *prefix, int *upper, int *has_sign)
{
    *size = 0;
    *prefix = 0;
    *upper = 0;
    *has_sign = 0;
    
    switch (token) {
    case 'd':
        *size = 8;
        *has_sign = 1;
        return __FT_INT64;
    case 'u':
        *size = 8;
        return __FT_INT64;
    case 'X':
        *upper = 1;
    case 'x':
        *size = 8;
        *prefix = 1;
        return __FT_HEX64;
    case 'H':
        *upper = 1;
    case 'h':
        *size = 8;
        return __FT_HEX64;
    case 'B':
    case 'b':
        *size = 8;
        return __FT_BIN64;
    default:
        return __FT_UNKNOWN;
    }
    
    return __FT_UNKNOWN;
}

static int find_token(char *tp, int *ft_count, int *size, int *prefix, int *upper, int *has_sign)
{
    *ft_count = 0;
    *size = 0;
    *prefix = 0;
    *upper = 0;
    *has_sign = 0;
    
    int ret_long = __FT_UNKNOWN;
    char token_long = 0;
    
    *ft_count = 1;
    
    switch (*tp) {
    case '%':
        return __FT_PERCENT;
    case 'c':
        *size = 4;
        return __FT_CHAR;
    case 'd':
        *size = 4;
        *has_sign = 1;
        return __FT_INT;
    case 'u':
        *size = 4;
        return __FT_INT;
    case 'X':
        *upper = 1;
    case 'x':
        *size = 4;
        *prefix = 1;
        return __FT_HEX;
    case 'H':
        *upper = 1;
    case 'h':
        *size = 4;
        return __FT_HEX;
    case 'B':
    case 'b':
        *size = 4;
        return __FT_BIN;
    case 'P':
        *upper = 1;
    case 'p':
        *prefix = 1;
        if (sizeof(u64) == sizeof(ulong)) {
            *size = 8;
            return __FT_HEX64;
        } else {
            *size = 4;
            return __FT_HEX;
        }
    case 's':
        *size = (sizeof(u64) == sizeof(ulong)) ? 8 : 4;
        return __FT_STR;
    
    case 'l':
        token_long = *(tp + 1);
        *ft_count = 2;
        
        if (token_long == 'l') {
            token_long = *(tp + 2);
            *ft_count = 3;
        }
        
        ret_long = find_long_token(token_long, size, prefix, upper, has_sign);
        if (__FT_UNKNOWN == ret_long) {
            *ft_count = 0;
        }
        
        return ret_long;
    default:
        return __FT_UNKNOWN;
    }
    
    return __FT_UNKNOWN;
}

#define POP_ARG4()  (*(u32 *)va); va += sizeof(u32)
#define POP_ARG8()  (*(u64 *)va); va += sizeof(u64)

int asmlinkage kprintf(char *fmt, ...)
{
    char *c = fmt;
    int ftype = 0, ft_count, param_size, prefix, upper, has_sign;
    
    u32 arg4;
    u64 arg8;
    void *va = &fmt + sizeof(char *);
    
    while (*c) {
        switch (*c) {
        case '%':
            // Find out token type
            ftype = find_token(c + 1, &ft_count, &param_size, &prefix, &upper, &has_sign);
            
            // Pop data from stack
            switch (param_size) {
            case 4:
                arg4 = POP_ARG4();
                break;
            case 8:
                arg8 = POP_ARG8();
                break;
            default:
                break;
            }
            
            // Print the data
            switch (ftype) {
            case __FT_INT:
                print_int(arg4, has_sign);
                break;
            case __FT_HEX:
                print_hex(arg4, prefix, upper);
                break;
            case __FT_BIN:
                print_bin(arg4);
                break;
            case __FT_INT64:
                print_int64(arg4, has_sign);
                break;
            case __FT_HEX64:
                print_hex64(arg4, prefix, upper);
                break;
            case __FT_BIN64:
                print_bin64(arg4);
                break;
            case __FT_CHAR:
                print_char((char)arg4);
                break;
            case __FT_STR:
                if (param_size == 4) {
                    print_string((char *)arg4);
                } else {
                    print_string((char *)arg8);
                }
                break;
            case __FT_PERCENT:
                print_char('%');
                break;
            case __FT_UNKNOWN:
            default:
                print_unknown();
                break;
            }
            
            c += ft_count;
            break;
        default:
            print_char(*c);
            break;
        }
        
        c++;
    }
    
}
