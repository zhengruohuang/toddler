#include "common/include/data.h"


#define ABS(x)  ((x) < 0 ? -(x) : (x))


void div_u32(u32 dividend, u32 divisor, u32 *quo_out, u32 *rem_out)
{
    u32 t, num_bits;
    u32 q, bit, d;
    int i;
    
    u32 remainder = 0;
    u32 quotient = 0;
    
    if (divisor == 0) {
        goto done;
    } else if (divisor > dividend) {
        remainder = dividend;
        goto done;
    } else if (divisor == dividend) {
        quotient = 1;
        goto done;
    }
    
    num_bits = 32;
    
    while (remainder < divisor) {
        bit = (dividend & 0x80000000) >> 31;
        remainder = (remainder << 1) | bit;
        d = dividend;
        dividend = dividend << 1;
        num_bits--;
    }
    
    dividend = d;
    remainder = remainder >> 1;
    num_bits++;
    
    for (i = 0; i < num_bits; i++) {
        bit = (dividend & 0x80000000) >> 31;
        remainder = (remainder << 1) | bit;
        t = remainder - divisor;
        q = !((t & 0x80000000) >> 31);
        dividend = dividend << 1;
        quotient = (quotient << 1) | q;
        if (q) {
            remainder = t;
        }
    }

done:
    if (quo_out) {
        *quo_out = quotient;
    }
    
    if (rem_out) {
        *rem_out = remainder;
    }
}

void div_s32(s32 dividend, s32 divisor, s32 *quo_out, s32 *rem_out)
{
    u32 dend, dor;
    u32 q, r;
    
    dend = ABS(dividend);
    dor  = ABS(divisor);
    div_u32(dend, dor, &q, &r);
    
    s32 quotient = q;
    s32 remainder = r;
    
    if (dividend < 0) {
        remainder = -r;
        if (divisor > 0) {
            quotient = -q;
        }
    } else {
        remainder = r;
        if (divisor < 0) {
            quotient = -q;
        }
    }
    
    if (quo_out) {
        *quo_out = quotient;
    }
    
    if (rem_out) {
        *rem_out = remainder;
    }
}

void div_int(int dividend, int divisor, int *quo_out, int *rem_out)
{
    div_s32(dividend, divisor, quo_out, rem_out);
}

void div_uint(uint dividend, uint divisor, uint *quo_out, uint *rem_out)
{
    div_u32(dividend, divisor, quo_out, rem_out);
}
