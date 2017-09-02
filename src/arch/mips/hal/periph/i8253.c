#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/periph.h"


static int pit_inuse = 0;
static int pit_as_tick = 0;


void pit_disable()
{
    struct i8253_cmd cmd;
    cmd.bcd = 0;
    cmd.channel = 0;
    cmd.command = I8253_CMD_ALL;
    cmd.mode = I8253_MODE_RETRIGGERABLE;
    
    io_write8(I8253_CMD_ADDR, cmd.value);
    io_write8(I8253_CH0_ADDR, (u8)255);
    io_write8(I8253_CH0_ADDR, (u8)255);
}

static int read_pit_count()
{
    // Latch
    struct i8253_cmd cmd;
    cmd.bcd = 0;
    cmd.mode = 0;
    cmd.command = I8253_CMD_LATCH;
    cmd.channel = 0;
    
    io_write8(I8253_CMD_ADDR, cmd.value);
    
    // Read
    u8 low = io_read8(I8253_CH0_ADDR);
    u8 high = io_read8(I8253_CH0_ADDR);
    
    // Concatinate
    int result = high;
    result <<= 8;
    result += low;
    
    return result;
}

void pit_delay(int ms)
{
    assert(ms > 0 && ms <= I8253_DELAY_MAX_MS);
    assert(0 == pit_inuse);
    
    // Claim the PIT
    pit_inuse = 1;
    
    // Calculate the count
    int count = ms * I8253_INPUT_FREQ / 1000 + I8253_DELAY_THRESHOLD;
    
    // Program the PIT
    struct i8253_cmd cmd;
    cmd.bcd = 0;
    cmd.channel = 0;
    cmd.command = I8253_CMD_ALL;
    cmd.mode = I8253_MODE_TERMINAL;
    
    io_write8(I8253_CMD_ADDR, cmd.value);
    io_write8(I8253_CH0_ADDR, (u8)count);
    io_write8(I8253_CH0_ADDR, (u8)(count >> 8));
    
    // Start polling the counter
    int lowest = I8253_INPUT_FREQ;
    int wait = 0;
    
    while (1) {
        int cur = read_pit_count();
        //kprintf("cur: %d\n", cur);
        if (cur < I8253_DELAY_THRESHOLD || cur > lowest || wait > I8253_DELAY_TIMEOUT) {
            break;
        }
        
        wait++;
        lowest = cur;
    }
    
    //halt();
    
    // Disable it to prevent it from messing up the system
    pit_disable();
    
    // Release the PIT
    pit_inuse = 0;
}

void pit_gen_tick(int freq)
{
    assert(!pit_inuse || pit_as_tick);
    
    // Claim the PIT
    pit_inuse = 1;
    pit_as_tick = 1;
    
    // Program the PIT
    struct i8253_cmd cmd;
    cmd.bcd = 0;
    cmd.channel = 0;
    cmd.command = I8253_CMD_ALL;
    cmd.mode = I8253_MODE_SQUARE;
    
    u16 count = 65535;
    
    io_write8(I8253_CMD_ADDR, cmd.value);
    io_write8(I8253_CH0_ADDR, (u8)(count & 0xff));
    io_write8(I8253_CH0_ADDR, (u8)(count >> 8));
}