#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/time.h"


static struct rtc_date_time rtc_date_time;
static int century_reg = 0x00;


static int check_update()
{
    io_out8(RTC_REGISTER_ADDRESS, 0x0A);
    return (io_in8(RTC_REGISTER_DATA) & 0x80);
}

static void wait_update()
{
    while (check_update()) {
        __asm__ __volatile__( "pause;" : : );
    }
}

static int read_rtc_reg(int target_register)
{
    io_out8(RTC_REGISTER_ADDRESS, target_register);
    int result = io_in8(RTC_REGISTER_DATA);
    return result;
}

void read_rtc(struct rtc_date_time *dt)
{
    int last_second;
    int last_minute;
    int last_hour;
    int last_day;
    int last_month;
    int last_year;
    int last_century;
    int registerB;
    
    if (!dt) {
        return;
    }
    
    /*
     * Note: This uses the "read registers until you get the same values twice in a row" technique
     *       to avoid getting dodgy/inconsistent values due to rtc updates
     */
    
    // Make sure an update isn't in progress
    wait_update();
    
    dt->second = read_rtc_reg(0x00);
    dt->minute = read_rtc_reg(0x02);
    dt->hour = read_rtc_reg(0x04);
    dt->day = read_rtc_reg(0x07);
    dt->month = read_rtc_reg(0x08);
    dt->year = read_rtc_reg(0x09);
    if (century_reg) {
        dt->century = read_rtc_reg(century_reg);
    }
    
    do {
        last_second = dt->second;
        last_minute = dt->minute;
        last_hour = dt->hour;
        last_day = dt->day;
        last_month = dt->month;
        last_year = dt->year;
        last_century = dt->century;
        
        // Make sure an update isn't in progress
        wait_update();
        
        dt->second = read_rtc_reg(0x00);
        dt->minute = read_rtc_reg(0x02);
        dt->hour = read_rtc_reg(0x04);
        dt->day = read_rtc_reg(0x07);
        dt->month = read_rtc_reg(0x08);
        dt->year = read_rtc_reg(0x09);
        if (century_reg) {
            dt->century = read_rtc_reg(century_reg);
        }
    } while (
        (last_second == dt->second) &&
        (last_minute == dt->minute) &&
        (last_hour == dt->hour) &&
        (last_day == dt->day) &&
        (last_month == dt->month) &&
        (last_year == dt->year) &&
        (last_century == dt->century)
    );
    
    registerB = read_rtc_reg(0x0B);
    
    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04)) {
        dt->second = (dt->second & 0x0F) + ((dt->second / 16) * 10);
        dt->minute = (dt->minute & 0x0F) + ((dt->minute / 16) * 10);
        dt->hour = ((dt->hour & 0x0F) + (((dt->hour & 0x70) / 16) * 10)) | (dt->hour & 0x80);
        dt->day = (dt->day & 0x0F) + ((dt->day / 16) * 10);
        dt->month = (dt->month & 0x0F) + ((dt->month / 16) * 10);
        dt->year = (dt->year & 0x0F) + ((dt->year / 16) * 10);
        if(century_reg) {
            dt->century = (dt->century & 0x0F) + ((dt->century / 16) * 10);
        }
    }
    
    // Convert 12 hour clock to 24 hour clock if necessary
    if (!(registerB & 0x02) && (dt->hour & 0x80)) {
        dt->hour = ((dt->hour & 0x7F) + 12) % 24;
    }
    
    // Calculate the full (4-digit) year
    if(century_reg) {
        dt->year += dt->century * 100;
    } else {
        if (dt->year > 80 && dt->year < 100) {
            dt->century = 20;
            dt->year += 1900;
        } else {
            dt->century = 21;
            dt->year += 2000;
        }
        //year += (CURRENT_YEAR / 100) * 100;
        //if (year < CURRENT_YEAR) year += 100;
    }
}

void init_rtc()
{
    read_rtc(&rtc_date_time);
    
    kprintf("Initializing real time clock\n");
    kprintf("\tCurrent Date Time: %d Century, %d-%d-%d %d:%d:%d\n",
            rtc_date_time.century,
            rtc_date_time.year, rtc_date_time.month, rtc_date_time.day,
            rtc_date_time.hour, rtc_date_time.minute, rtc_date_time.second
    );
}
