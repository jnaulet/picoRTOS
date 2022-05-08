#include "picoRTOS_cond.h"

#include "picoRTOS_spinlock.h"

void picoRTOS_cond_init(struct picoRTOS_cond *cond)
{
    cond->act = PICORTOS_COND_NONE;
    cond->count = (picoRTOS_size_t)0;
}

void picoRTOS_cond_signal(struct picoRTOS_cond *cond)
{
    arch_spin_lock1();
    cond->act = PICORTOS_COND_SIGNAL;
    ASM("dmb ish");
    arch_spin_unlock1();
}

void picoRTOS_cond_broadcast(struct picoRTOS_cond *cond)
{
    arch_spin_lock1();
    cond->act = PICORTOS_COND_BROADCAST;
    ASM("dmb ish");
    arch_spin_unlock1();
}

void picoRTOS_cond_wait(struct picoRTOS_cond *cond, struct picoRTOS_mutex *mutex)
{
    arch_assert(cond->count < (picoRTOS_size_t)CONFIG_TASK_COUNT);

    /* we already own the mutex */
    arch_spin_lock1();
    cond->count++;
    ASM("dmb ish");
    arch_spin_unlock1();

    for (;;) {
        if (cond->act != PICORTOS_COND_NONE)
            break;

        picoRTOS_mutex_unlock(mutex);
        picoRTOS_schedule();
        picoRTOS_mutex_lock(mutex);
    }

    /* reset */
    arch_spin_lock1();
    if (--cond->count == 0 || cond->act == PICORTOS_COND_SIGNAL)
        cond->act = PICORTOS_COND_NONE;
    ASM("dmb ish");
    arch_spin_unlock1();
}
