#include "common/include/data.h"
#include "loader/include/print.h"


/*
 * Arch-specific draw char
 */
extern void draw_char(char ch);


/*
 * Print
 */
static void print_char(char ch)
{
    draw_char(ch);
}

static void print_string(char *str)
{
    char *s = str;
    
    while (*s) {
        print_char(*s);
        s++;
    }
}

static void div_u32(u32 a, u32 b, u32 *qout, u32 *rout)
{
    u32 q = 0, r = a;
    
    while (r >= b) {
        q++;
        r -= b;
    }
    
    if (qout) {
        *qout = q;
    }
    
    if (rout) {
        *rout = r;
    }
}

static void div_u64(u64 a, u64 b, u64 *qout, u64 *rout)
{
    u64 q = 0, r = a;
    
    while (r >= b) {
        q++;
        r -= b;
    }
    
    if (qout) {
        *qout = q;
    }
    
    if (rout) {
        *rout = r;
    }
}

static void print_num_u32(char fmt, u32 num)
{
    int i;
    int started = 0;
    
    switch (fmt) {
    case 'b':
        for (i = 31; i >= 0; i--) {
            print_char(num & (0x1 << i) ? '1' : '0');
        }
        break;
    case 'x':
        print_string("0x");
    case 'h':
        if (!num) {
            print_char('0');
        } else {
            for (i = 0; i < sizeof(u32) * 8; i += 4) {
                u32 value = (num << i) >> 28;
                if (value) {
                    started = 1;
                }
                if (started) {
                    print_char(value > 9 ?
                        (char)(value - 10 + 'a') :
                        (char)(value + '0'));
                }
            }
        }
        break;
    case 'd':
        if (num & (0x1 << 31)) {
            print_char('-');
            num = ~num + 1;
        }
    case 'u':
        if (!num) {
            print_char('0');
            break;
        } else {
            int digits = 10;
            u32 dividers[] = { 1000000000, 100000000, 10000000,
                1000000, 100000, 10000, 1000, 100, 10, 1 };
            u32 q, r;
            
            for (i = 0; i < digits; i++) {
                div_u32(num, dividers[i], &q, &r);
                
                if (q) {
                    started = 1;
                }
                
                if (started) {
                    print_char('0' + q);
                }
                
                num = r;
            }
        }
        break;
    default:
        print_string("__unknown_format_");
        print_char(fmt);
        print_string("__");
        break;
    }
}

static void print_num_u64(char fmt, u64 num)
{
    int i;
    int started = 0;
    
    switch (fmt) {
    case 'b':
        for (i = 63; i >= 0; i--) {
            print_char(num & (0x1 << i) ? '1' : '0');
        }
        break;
    case 'x':
        print_string("0x");
    case 'h':
        if (!num) {
            print_char('0');
        } else {
            int started = 0;
            for (i = 0; i < sizeof(u64) * 8; i += 4) {
                u64 value = (num << i) >> 60;
                if (value) {
                    started = 1;
                }
                if (started) {
                    print_char(value > 9 ?
                        (char)(value - 10 + 'a') :
                        (char)(value + '0'));
                }
            }
        }
        break;
    case 'd':
        if (num & (0x1ull << 63)) {
            print_char('-');
            num = ~num + 0x1ull;
        }
    case 'u':
        if (!num) {
            print_char('0');
            break;
        } else {
            int digits = 20;
            u64 dividers[] = { 0x8AC7230489E80000ull,
                0xDE0B6B3A7640000ull, 0x16345785D8A0000ull,
                0x2386F26FC10000ull, 0x38D7EA4C68000ull,
                0x5AF3107A4000ull, 0x9184E72A000ull,
                0xE8D4A51000ull, 0x174876E800ull, 0x2540BE400ull,
                1000000000, 100000000, 10000000,
                1000000, 100000, 10000, 1000, 100, 10, 1 };
            u64 q, r;
            
            for (i = 0; i < digits; i++) {
                div_u64(num, dividers[i], &q, &r);
                
                if (q) {
                    started = 1;
                }
                
                if (started) {
                    print_char('0' + q);
                }
                
                num = r;
            }
        }
        break;
    default:
        print_string("__unknown_format_");
        print_char(fmt);
        print_string("__");
        break;
    }
}

static void print_num_ulong(char fmt, ulong num)
{
#if ARCH_WIDTH == 32
    print_num_u32(fmt, (u32)num);
#else
    print_num_u64(fmt, (u64)num);
#endif
}

void lprintf(char *fmt, ...)
{
    char *s = fmt;
    char token = 0;
    
    char *va_str;
    char va_ch;
    u32 va_u32;
    ulong va_ulong;
    u64 va_u64;
    
    va_list va;
    va_start(va, fmt);
    
    while (*s) {
        switch (*s) {
        case '%':
            s++;
            token = *s;
            
            switch (token) {
            case 'd':
            case 'u':
            case 'x':
                va_u32 = va_arg(va, u32);
                print_num_u32(token, va_u32);
                break;
            case 'l':
                s++;
                token = *s;
                if (!token) {
                    print_string("%l");
                    break;
                }
                
                switch (token) {
                case 'd':
                case 'u':
                case 'x':
                    va_ulong = va_arg(va, ulong);
                    print_num_ulong(token, va_ulong);
                    break;
                case 'l':
                    s++;
                    token = *s;
                    if (!token) {
                        print_string("%ll");
                        break;
                    }
                    
                    switch (token) {
                    case 'd':
                    case 'u':
                    case 'x':
                        va_u64 = va_arg(va, u64);
                        print_num_u64(token, va_u64);
                        break;
                    default:
                        print_string("%ll");
                        print_char(token);
                        break;
                    }
                default:
                    print_string("%l");
                    print_char(token);
                    break;
                }
                break;
            case 's':
                va_str = va_arg(va, char *);
                print_string(va_str);
                break;
            case 'c':
                va_ch = va_arg(va, int);
                print_char(va_ch);
                break;
            case 'p':
                va_ulong = va_arg(va, ulong);
                print_num_ulong('x', va_ulong);
                break;
            case '%':
                print_char(token);
                break;
            default:
                print_char('%');
                print_char(token);
                break;
            }
            
            break;
        default:
            print_char(*s);
            break;
        }
        
        s++;
    }
    
    va_end(va);
}
