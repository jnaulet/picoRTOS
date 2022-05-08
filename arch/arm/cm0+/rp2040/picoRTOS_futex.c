#include "picoRTOS_spinlock.h"

#include "picoRTOS_futex.h"

/* check */
#ifndef CONFIG_DEADLOCK_COUNT
# error no deadlock defined for locks
#endif

void picoRTOS_futex_init(picoRTOS_futex_t *futex)
{
    *futex = (picoRTOS_futex_t)0;
}

int picoRTOS_futex_trylock(picoRTOS_futex_t *futex)
{
    int r;
    arch_spin_lock1();
    if (*futex == 0) {
        *futex = 1;
        r = 0;
    }
    else
        r = -1;
    ASM("dmb ish");
    arch_spin_unlock1();

    return r;
}

void picoRTOS_futex_lock(picoRTOS_futex_t *futex)
{
    int loop = CONFIG_DEADLOCK_COUNT;

    while (picoRTOS_futex_trylock(futex) != 0 && loop-- != 0)
        picoRTOS_schedule();

    /* check for deadlock */
    arch_assert(loop != -1);
}

void picoRTOS_futex_unlock(picoRTOS_futex_t *futex)
{
    arch_spin_lock1();
    *futex = (picoRTOS_futex_t)0;
    ASM("dmb ish");
    arch_spin_unlock1();
}
