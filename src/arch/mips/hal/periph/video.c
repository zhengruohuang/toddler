#include "common/include/data.h"
#include "hal/include/periph.h"


static int is_ascii(char ch)
{
    return ch >= 32 && ch <= 126;
}


void draw_char(char ch)
{
    if (ch == '\n' || ch == '\t' || ch == '\b' || is_ascii(ch)){
        uart_write(ch);
    } else {
        uart_write('?');
    }
    
//     if (!is_ascii(ch)) {
//         return;
//     }
//     
//     uart_write(ch);
}

void init_video()
{
}
