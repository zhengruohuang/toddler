#include "common/include/data.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdarg.h"


#define BUF_SIZE        128

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


static void print_char(char *buf, size_t size, int *index, char c)
{
    int idx = *index;
    
    if (idx >= size - 1) {
        buf[size - 1] = '\0';
        return;
    }
    
    buf[idx] = c;
    *index = idx + 1;
}

static void print_string(char *buf, size_t size, int *index, char *s)
{
    char *c = s;
    
    while (*c) {
        print_char(buf, size, index, *c++);
    }
}

static void print_bin64(char *buf, size_t size, int *index, u64 value)
{
    int i;
    
    for (i = 63; i >= 0; i++) {
        print_char(buf, size, index, (value & (0x1ull << i)) ? '1' : '0');
    }
}

static void print_bin(char *buf, size_t size, int *index, u32 value)
{
    int i;
    
    for (i = 31; i >= 0; i++) {
        print_char(buf, size, index, (value & (0x1 << i)) ? '1' : '0');
    }
}

static void print_hex64(char *buf, size_t size, int *index, u64 value, int prefix, int upper)
{
    int i;
    u64 cur = 0;
    int started = 0;
    
    if (prefix) {
        print_string(buf, size, index, "0x");
    }
    
    for (i = 60; i >= 0; i -= 4) {
        cur = (value >> i) & 0xfull;
        
        if (cur >= (u64)10) {
            print_char(buf, size, index, (upper ? 'A' : 'a') + (int)cur - 10);
            started = 1;
        } else if (cur || started) {
            print_char(buf, size, index, '0' + (int)cur);
            started = 1;
        }
    }
    
    if (!started) {
        print_char(buf, size, index, '0');
    }
}

static void print_hex(char *buf, size_t size, int *index, u32 value, int prefix, int upper)
{
    int i;
    u32 cur = 0;
    int started = 0;
    
    if (prefix) {
        print_string(buf, size, index, "0x");
    }
    
    for (i = 28; i >= 0; i -= 4) {
        cur = (value >> i) & 0xf;
        
        if (cur >= (u32)10) {
            print_char(buf, size, index, (upper ? 'A' : 'a') + (int)cur - 10);
            started = 1;
        } else if (cur || started) {
            print_char(buf, size, index, '0' + (int)cur);
            started = 1;
        }
    }
    
    if (!started) {
        print_char(buf, size, index, '0');
    }
}

static void print_int64(char *buf, size_t size, int *index, u64 value, int has_sign)
{
    print_string(buf, size, index, "__UNSUPPORT_INT64__");
}

static void print_int(char *buf, size_t size, int *index, u32 value, int has_sign)
{
    u32 div = 1000000000;
    u32 cur = 0;
    int started = 0;
    
    if (has_sign && value >> 31) {
        print_string(buf, size, index, "-");
        value = ~value + 1;
    }
    
    while (div) {
        cur = value / div;
        
        if (cur || started) {
            print_char(buf, size, index, '0' + (int)cur);
            started = 1;
        }
        
        value %= div;
        div /= 10;
    }
    
    if (!started) {
        print_char(buf, size, index, '0');
    }
}

static void print_unknown(char *buf, size_t size, int *index)
{
    print_string(buf, size, index, " __UNKNOWN__ ");
}

static int find_long_token(char token, int *size, int *prefix, int *upper, int *has_sign)
{
    int is_long64 = sizeof(unsigned long) == sizeof(unsigned long long);
    
    *size = is_long64 ? 8 : 4;
    *prefix = 0;
    *upper = 0;
    *has_sign = 0;
    
    switch (token) {
    case 'd':
        *has_sign = 1;
        return is_long64 ? __FT_INT64 : __FT_INT;
    case 'u':
        return is_long64 ? __FT_INT64 : __FT_INT;
    case 'X':
        *upper = 1;
    case 'x':
        *prefix = 1;
        return is_long64 ? __FT_HEX64 : __FT_HEX;
    case 'H':
        *upper = 1;
    case 'h':
        return is_long64 ? __FT_HEX64 : __FT_HEX;
    case 'B':
    case 'b':
        return is_long64 ? __FT_BIN64 : __FT_BIN;
    default:
        *size = 0;
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

// #define POP_ARG4()  (*(u32 *)va); va += sizeof(u32)
// #define POP_ARG8()  (*(u64 *)va); va += sizeof(u64)

int vsnprintf(char *buf, size_t size, char *fmt, va_list ap)
{
    char *c = fmt;
    int ftype, ft_count, param_size, prefix, upper, has_sign;
    
    u32 arg4 = 0;
    u64 arg8 = 0;
    ulong va = (ulong)&fmt + sizeof(char *);
    
    int cur_index = 0;
    
    while (*c) {
        switch (*c) {
        case '%':
            // Find out token type
            ftype = find_token(c + 1, &ft_count, &param_size, &prefix, &upper, &has_sign);
            
            // Pop data from stack
            switch (param_size) {
            case 4:
                arg4 = va_arg(ap, u32);
                break;
            case 8:
                arg8 = va_arg(ap, u64);
                break;
            default:
                break;
            }
            
            // Print the data
            switch (ftype) {
            case __FT_INT:
                print_int(buf, size, &cur_index, arg4, has_sign);
                break;
            case __FT_HEX:
                print_hex(buf, size, &cur_index, arg4, prefix, upper);
                break;
            case __FT_BIN:
                print_bin(buf, size, &cur_index, arg4);
                break;
            case __FT_INT64:
                print_int64(buf, size, &cur_index, arg8, has_sign);
                break;
            case __FT_HEX64:
                print_hex64(buf, size, &cur_index, arg8, prefix, upper);
                break;
            case __FT_BIN64:
                print_bin64(buf, size, &cur_index, arg8);
                break;
            case __FT_CHAR:
                print_char(buf, size, &cur_index, (char)arg4);
                break;
            case __FT_STR:
                if (param_size == 4) {
                    print_string(buf, size, &cur_index, (char *)arg4);
                } else {
                    print_string(buf, size, &cur_index, (char *)(unsigned long)arg8);
                }
                break;
            case __FT_PERCENT:
                print_char(buf, size, &cur_index, '%');
                break;
            case __FT_UNKNOWN:
            default:
                print_unknown(buf, size, &cur_index);
                break;
            }
            
            c += ft_count;
            break;
                default:
                    print_char(buf, size, &cur_index, *c);
                    break;
        }
        
        c++;
    }
    
    // Finalize
    if (cur_index >= (int)size - 1) {
        cur_index = (int)size - 1;
    }
    buf[cur_index] = 0;
    
    return cur_index;
}
