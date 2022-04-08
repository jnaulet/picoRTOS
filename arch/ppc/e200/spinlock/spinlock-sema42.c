#include "picoRTOSConfig.h"
#include "picoRTOS_types.h"

#ifndef CONFIG_ARCH_PPC_E200_SMP_LOCK
# error Lock not defined in configuration !
#endif

#define SEMA42_GATE0 ((volatile unsigned char*)CONFIG_ARCH_PPC_E200_SMP_LOCK)
#define SEMA42_RSTGT ((volatile unsigned short*)(CONFIG_ARCH_PPC_E200_SMP_LOCK + 0x40))

/*@external@*/ extern picoRTOS_core_t arch_core(void);

void arch_spinlock_init(void)
{
    /* reset gate0 */
    *SEMA42_RSTGT = (unsigned short)0xe200;
    *SEMA42_RSTGT = (unsigned short)0x1d00;
}

void arch_spin_lock(void)
{
    int n = 0xffff;
    picoRTOS_core_t core = arch_core() + 1;

    do
        *SEMA42_GATE0 = (unsigned char)core;
    while (*SEMA42_GATE0 != (unsigned char)core &&
           n-- != 0);

    /* potential deadlock */
    arch_assert(n != -1);
}

void arch_spin_unlock(void)
{
    picoRTOS_core_t core = arch_core() + 1;

    arch_assert(core == (picoRTOS_core_t)*SEMA42_GATE0);
    *SEMA42_GATE0 = (unsigned char)0;
}
