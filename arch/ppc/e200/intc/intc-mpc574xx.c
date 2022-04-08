#include "picoRTOSConfig.h"
#include "picoRTOS_types.h"
#include "picoRTOS_port.h"

/* ASM */
/*@external@*/ extern void arch_TICK(void);

#define INTC_BCR   ((volatile unsigned long*)CONFIG_ARCH_PPC_E200_INTC)
#define INTC_CPR   ((volatile unsigned long*)(CONFIG_ARCH_PPC_E200_INTC + 0x10))
#define INTC_IACKR ((volatile unsigned long*)CONFIG_ARCH_PPC_E200_INTC_IACKR)
#define INTC_PSR   ((volatile unsigned short*)(CONFIG_ARCH_PPC_E200_INTC + 0x60))

void arch_intc_init(void)
{
    unsigned long *VTBA = (unsigned long*)(*INTC_IACKR & 0xfffff000);

    *INTC_BCR = 0;
    INTC_CPR[0] = 0;

#if defined CONFIG_SMP_CORES
    picoRTOS_size_t n = CONFIG_SMP_CORES;

    while (n-- != 0) {
        INTC_CPR[n] = 0;
        INTC_IACKR[n] = (unsigned long)VTBA;
    }
#endif

    /* TICK */
    VTBA[CONFIG_ARCH_PPC_E200_TIMER_IRQ] = (unsigned long)arch_TICK;

    /* priority 1 on any core */
    INTC_PSR[CONFIG_ARCH_PPC_E200_TIMER_IRQ] = (unsigned short)0xf001u;
}
