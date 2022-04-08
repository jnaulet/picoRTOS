#include "picoRTOS-SMP.h"
#include "picoRTOS-SMP_port.h"

void arch_smp_init(void)
{
    /* ARCH */
    arch_init();

    /* SPINLOCK */
    arch_spinlock_init();
}

void arch_memory_barrier(void)
{
    __asm__ volatile ("mbar");
}
