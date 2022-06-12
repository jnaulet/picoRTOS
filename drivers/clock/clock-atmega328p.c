#include "picoRTOS.h"
#include "clock-atmega328p.h"

clock_freq_t clock_get_freq(clock_id_t clkid)
{
    arch_assert(clkid < CLOCK_ATMEGA328P_COUNT);

    /* basically */
    return (clock_freq_t)CONFIG_SYSCLK_HZ;
}
