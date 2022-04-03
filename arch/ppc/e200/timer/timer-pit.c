#include "picoRTOSConfig.h"
#include "picoRTOS_types.h"
#include "picoRTOS_port.h"

#define PIT_MCR     ((volatile unsigned long*)CONFIG_ARCH_PPC_E200_TIMER)
/* channel 3 */
#define PIT_LDVAL3  ((volatile unsigned long*)(CONFIG_ARCH_PPC_E200_TIMER + 0x130))
#define PIT_CVAL3   ((volatile unsigned long*)(CONFIG_ARCH_PPC_E200_TIMER + 0x134))
#define PIT_TCTRL3  ((volatile unsigned long*)(CONFIG_ARCH_PPC_E200_TIMER + 0x138))
#define PIT_TFLG3   ((volatile unsigned long*)(CONFIG_ARCH_PPC_E200_TIMER + 0x13c))

void arch_timer_init(void)
{
    *PIT_MCR &= ~0x2;       /* enable PIT */
    *PIT_LDVAL3 = (unsigned long)(CONFIG_SYSCLK_HZ / CONFIG_TICK_HZ) - 1ul;
    *PIT_TCTRL3 |= 0x3;     /* enable interrupt & start */
}

void arch_timer_isr(void)
{
    *PIT_TFLG3 = 0x1ul;
}
