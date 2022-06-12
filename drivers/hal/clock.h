#ifndef HAL_CLOCK_H
#define HAL_CLOCK_H

typedef long clock_freq_t;
typedef unsigned int clock_id_t;

#define CLOCK_SYSCLK (clock_id_t)0 /* always */

clock_freq_t clock_get_freq(clock_id_t clkid);

#endif
