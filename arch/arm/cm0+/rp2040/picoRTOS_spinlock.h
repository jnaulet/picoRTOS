#ifndef PICORTOS_SPINLOCK_H
#define PICORTOS_SPINLOCK_H

#include "picoRTOS.h"

/* A word of caution:
 * This is not very useful and even dangerous in a non-SMP
 * environment. Prefer futexes if possible
 */

typedef picoRTOS_atomic_t picoRTOS_spinlock_t;

void picoRTOS_spinlock_init(/*@notnull@*/ /*@out@*/ picoRTOS_spinlock_t *lock);

int picoRTOS_spin_trylock(/*@notnull@*/ picoRTOS_spinlock_t *lock);
void picoRTOS_spin_lock(/*@notnull@*/ picoRTOS_spinlock_t *lock);
void picoRTOS_spin_unlock(/*@notnull@*/ picoRTOS_spinlock_t *lock);

#ifndef SIO_BASE
# define SIO_BASE 0xd0000000
#endif

#define SIO_SPINLOCK1 ((volatile unsigned long*)(SIO_BASE + 0x104))

static void arch_spin_lock1(void)
{
    int loop = CONFIG_DEADLOCK_COUNT;

    /* spins full throttle */
    while (*SIO_SPINLOCK1 == 0 && loop-- != 0) {
    }

    /* potential deadlock */
    arch_assert(loop != -1);
}

static void arch_spin_unlock1(void)
{
    *SIO_SPINLOCK1 = 1ul;
}

#endif
