#ifndef __MACH_RASPI2_HAL_INCLUDE_PERIPH__
#define __MACH_RASPI2_HAL_INCLUDE_PERIPH__


extern void init_periph();
extern void start_periph();

extern int periph_get_irq_vector();
extern int periph_get_fiq_vector();

extern int periph_detect_num_cpus();
extern void periph_waekup_cpu(int cpu_id, ulong entry);

#endif
