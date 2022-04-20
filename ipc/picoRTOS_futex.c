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
    if (arch_test_and_set((picoRTOS_atomic_t*)futex) != 0)
        return -1;

    return 0;
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
    *futex = (picoRTOS_futex_t)0;
}
