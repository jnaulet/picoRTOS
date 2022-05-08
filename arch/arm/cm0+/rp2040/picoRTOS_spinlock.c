#include "picoRTOS_spinlock.h"

/* check */
#ifndef CONFIG_DEADLOCK_COUNT
# error no deadlock defined for locks
#endif

void picoRTOS_spinlock_init(picoRTOS_spinlock_t *lock)
{
    *lock = (picoRTOS_spinlock_t)0;
}

int picoRTOS_spin_trylock(picoRTOS_spinlock_t *lock)
{
    int r;
    arch_spin_lock1();
    if (*lock == 0) {
        *lock = 1;
        r = 0;
    }
    else
        r = -1;
    ASM("dmb ish");
    arch_spin_unlock1();
    return r;
}

void picoRTOS_spin_lock(picoRTOS_spinlock_t *lock)
{
    int loop = CONFIG_DEADLOCK_COUNT;

    /* spins full throttle */
    while (picoRTOS_spin_trylock(lock) != 0 && loop-- != 0) {
        /* NOP */
    }

    /* check for deadlock */
    arch_assert(loop != -1);
}

void picoRTOS_spin_unlock(picoRTOS_spinlock_t *lock)
{
    arch_spin_lock1();
    *lock = (picoRTOS_spinlock_t)0;
    ASM("dmb ish");
    arch_spin_unlock1();
}
