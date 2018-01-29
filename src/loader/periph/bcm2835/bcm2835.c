#include "common/include/data.h"
#include "loader/include/periph/bcm2835.h"


// RPi 2
// Base @ 0x3F000000ul
//  End @ 0x3FFFFFFFul

void init_bcm2835(ulong bcm2835_base, ulong bcm2835_end)
{
    init_bcm2835_timer(bcm2835_base);
    init_bcm2835_mailbox(bcm2835_base);
    init_bcm2835_gpio(bcm2835_base);
//     init_bcm2835_uart(bcm2835_base);
    init_bcm2835_pl011(bcm2835_base);
    init_bcm2835_led();
    init_bcm2835_framebuffer(bcm2835_end);
    
    bcm2835_maximize_cpu_clock();
}


/*
void hardware_init(void) {
	int32_t board_revision;

#if defined ( RPI2 ) || defined ( RPI3 )
#ifndef ARM_ALLOW_MULTI_CORE
	// put all secondary cores to sleep
	uint8_t core_number = 1;
	for (core_number = 1 ; core_number < 4; core_number ++) {
		*(uint32_t *) (SMP_CORE_BASE + (core_number * 0x10)) = (uint32_t) _init_core;
	}
#endif
#endif
	(void) console_init();

	hardware_init_startup_micros = bcm2835_st_read();

	sys_time_init();

	bcm2835_rng_init();

	(void) bcm2835_vc_set_power_state(BCM2835_VC_POWER_ID_SDCARD, BCM2835_VC_SET_POWER_STATE_ON_WAIT);


}
*/
