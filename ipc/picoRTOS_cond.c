#include "picoRTOS_cond.h"

void picoRTOS_cond_init(struct picoRTOS_cond *cond)
{
    cond->act = PICORTOS_COND_NONE;
    cond->count = (picoRTOS_size_t)0;
}

void picoRTOS_cond_signal(struct picoRTOS_cond *cond)
{
    cond->act = PICORTOS_COND_SIGNAL;
}

void picoRTOS_cond_broadcast(struct picoRTOS_cond *cond)
{
    cond->act = PICORTOS_COND_BROADCAST;
}

void picoRTOS_cond_wait(struct picoRTOS_cond *cond, struct picoRTOS_mutex *mutex)
{
    arch_assert(cond->count < (picoRTOS_size_t)CONFIG_TASK_COUNT);

    /* we already own the mutex */
    cond->count++;

    for (;;) {
        if (cond->act != PICORTOS_COND_NONE)
            break;

        picoRTOS_mutex_unlock(mutex);
        picoRTOS_schedule();
        picoRTOS_mutex_lock(mutex);
    }

    /* reset */
    if (--cond->count == 0 ||
        cond->act == PICORTOS_COND_SIGNAL)
        cond->act = PICORTOS_COND_NONE;
}
