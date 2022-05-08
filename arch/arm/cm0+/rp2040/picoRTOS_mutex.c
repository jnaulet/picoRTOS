#include "picoRTOS_spinlock.h"

#include "picoRTOS_mutex.h"

/* check */
#ifndef CONFIG_DEADLOCK_COUNT
# error no deadlock defined for locks
#endif

void picoRTOS_mutex_init(struct picoRTOS_mutex *mutex)
{
    mutex->owner = (picoRTOS_atomic_t)-1;
    mutex->count = 0;
}

int picoRTOS_mutex_trylock(struct picoRTOS_mutex *mutex)
{
    picoRTOS_atomic_t unused = (picoRTOS_atomic_t)-1;
    picoRTOS_atomic_t prio = (picoRTOS_atomic_t)picoRTOS_self();
    arch_spin_lock1();
    /* mutex is re-entrant */
    if (mutex->owner != (picoRTOS_atomic_t)-1 && mutex->owner != prio) {
        arch_spin_unlock1();
        return -1;
    }
    mutex->owner = prio;
    mutex->count++;
    ASM("dmb ish");
    arch_spin_unlock1();
    return 0;
}

void picoRTOS_mutex_lock(struct picoRTOS_mutex *mutex)
{
    int loop = CONFIG_DEADLOCK_COUNT;

    while (picoRTOS_mutex_trylock(mutex) != 0 && loop-- != 0)
        picoRTOS_schedule();

    /* deadlock ? */
    arch_assert(loop != -1);
}

void picoRTOS_mutex_unlock(struct picoRTOS_mutex *mutex)
{
    arch_assert(mutex->owner == (picoRTOS_atomic_t)picoRTOS_self());
    arch_assert(mutex->count > 0);

    arch_spin_lock1();
    if (--mutex->count == 0)
        mutex->owner = (picoRTOS_atomic_t)-1;
    ASM("dmb ish");
    arch_spin_unlock1();
}
