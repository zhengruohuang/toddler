#include "common/include/data.h"
#include "hal/include/periph.h"


void draw_char(char ch)
{
    uart_write(ch);
}

void init_video()
{
}
