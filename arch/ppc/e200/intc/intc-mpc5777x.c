#include "picoRTOSConfig.h"
#include "picoRTOS_types.h"

/* CHECK FOR ERRORS */
#ifndef CONFIG_ARCH_PPC_E200_INTC
# error Interrupt controller address is not defined !
#endif
#ifndef CONFIG_ARCH_PPC_E200_INTC_IACKR
# error Interrupt controller IACKR address is not defined !
#endif
#ifndef CONFIG_ARCH_PPC_E200_TIMER_IRQ
# error Tick timer IRQ is not set !
#endif

/* ASM */
/*@external@*/ extern void arch_TICK(void);

#define INTC_MCR   ((volatile unsigned long*)CONFIG_ARCH_PPC_E200_INTC)
#define INTC_CPR   ((volatile unsigned long*)(CONFIG_ARCH_PPC_E200_INTC + 0x8))
#define INTC_IACKR ((volatile unsigned long*)CONFIG_ARCH_PPC_E200_INTC_IACKR)
#define INTC_PSR   ((volatile unsigned char*)(CONFIG_ARCH_PPC_E200_INTC + 0x40))

void arch_intc_init(void)
{
    unsigned long *VTBA = (unsigned long*)(*INTC_IACKR & 0xfffff000);

    *INTC_MCR = 0;
    *INTC_CPR = 0;

    /* TICK */
    VTBA[CONFIG_ARCH_PPC_E200_TIMER_IRQ] = (unsigned long)arch_TICK;

    /* priority 1 on any core */
    INTC_PSR[CONFIG_ARCH_PPC_E200_TIMER_IRQ] = (unsigned char)0xc1u;
}
