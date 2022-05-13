#include "picoRTOS_fcond.h"

void picoRTOS_fcond_init(struct picoRTOS_fcond *fcond)
{
    fcond->act = PICORTOS_FCOND_NONE;
    fcond->count = (picoRTOS_size_t)0;
}

void picoRTOS_fcond_signal(struct picoRTOS_fcond *fcond)
{
    fcond->act = PICORTOS_FCOND_SIGNAL;
}

void picoRTOS_fcond_broadcast(struct picoRTOS_fcond *fcond)
{
    fcond->act = PICORTOS_FCOND_BROADCAST;
}

void picoRTOS_fcond_wait(struct picoRTOS_fcond *fcond, picoRTOS_futex_t *futex)
{
    arch_assert(fcond->count < (picoRTOS_size_t)CONFIG_TASK_COUNT);

    /* we already own the futex */
    fcond->count++;

    for (;;) {
        if (fcond->act != PICORTOS_FCOND_NONE)
            break;

        picoRTOS_futex_unlock(futex);
        picoRTOS_schedule();
        picoRTOS_futex_lock(futex);
    }

    /* reset */
    if (--fcond->count == 0 ||
        fcond->act == PICORTOS_FCOND_SIGNAL)
        fcond->act = PICORTOS_FCOND_NONE;
}
