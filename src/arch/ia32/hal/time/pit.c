#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/time.h"


static int pit_inuse = 0;
static int pit_as_tick = 0;


void disable_pit()
{
    struct pit_cmd cmd;
    cmd.bcd = 0;
    cmd.channel = 0;
    cmd.command = PIT_CMD_ALL;
    cmd.mode = PIT_MODE_RETRIGGERABLE;
    
    io_out8(PIT_CMD_PORT, cmd.value);
    io_out8(PIT_CH0_PORT, (u8)255);
    io_out8(PIT_CH0_PORT, (u8)255);
}

static int read_pit_count()
{
    // Latch
    struct pit_cmd cmd;
    cmd.bcd = 0;
    cmd.mode = 0;
    cmd.command = PIT_CMD_LATCH;
    cmd.channel = 0;
    
    io_out8(PIT_CMD_PORT, cmd.value);
    
    // Read
    u8 low = io_in8(PIT_CH0_PORT);
    u8 high = io_in8(PIT_CH0_PORT);
    
    // Concatinate
    int result = high;
    result <<= 8;
    result += low;
    
    //hal_printf("\n%d, %d\n", (u32)high, (u32)low);
    
    return result;
}

void pit_delay(int ms)
{
    assert(ms > 0 && ms <= PIT_DELAY_MAX_MS);
    assert(0 == pit_inuse);
    
    // Claim the PIT
    pit_inuse = 1;
    
    // Calculate the count
    int count = ms * PIT_INPUT_FREQ / 1000 + PIT_DELAY_THRESHOLD;
    
    // Program the PIT
    struct pit_cmd cmd;
    cmd.bcd = 0;
    cmd.channel = 0;
    cmd.command = PIT_CMD_ALL;
    cmd.mode = PIT_MODE_TERMINAL;
    
    io_out8(PIT_CMD_PORT, cmd.value);
    io_out8(PIT_CH0_PORT, (u8)count);
    io_out8(PIT_CH0_PORT, (u8)(count >> 8));
    
    // Start polling the counter
    int lowest = PIT_INPUT_FREQ;
    int wait = 0;
    
    while (1) {
        int cur = read_pit_count();
        //kprintf("cur: %d\n", cur);
        if (cur < PIT_DELAY_THRESHOLD || cur > lowest || wait > PIT_DELAY_TIMEOUT) {
            break;
        }
        
        wait++;
        lowest = cur;
    }
    
    //halt();
    
    // Disable the PTI to prevent it from messing up the system
    disable_pit();
    
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
    struct pit_cmd cmd;
    cmd.bcd = 0;
    cmd.channel = 0;
    cmd.command = PIT_CMD_ALL;
    cmd.mode = PIT_MODE_RATE;
    
    int count = PIT_INPUT_FREQ / freq;
    
    io_out8(PIT_CMD_PORT, cmd.value);
    io_out8(PIT_CH0_PORT, (u8)count);
    io_out8(PIT_CH0_PORT, (u8)(count >> 8));
}
