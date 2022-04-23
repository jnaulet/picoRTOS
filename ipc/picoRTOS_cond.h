#ifndef PICORTOS_COND_H
#define PICORTOS_COND_H

#include "picoRTOS.h"
#include "picoRTOS_mutex.h"

enum picoRTOS_cond_act {
    PICORTOS_COND_NONE,
    PICORTOS_COND_SIGNAL,
    PICORTOS_COND_BROADCAST
};

struct picoRTOS_cond {
    enum picoRTOS_cond_act act;
    picoRTOS_size_t count;
};

/* macro for static init */
#define PICORTOS_COND_INITIALIZER               \
    { PICORTOS_COND_NONE, (picoRTOS_size_t)0 }

void picoRTOS_cond_init(/*@notnull@*/ struct picoRTOS_cond *cond);

void picoRTOS_cond_signal(/*@notnull@*/ struct picoRTOS_cond * cond);                                   /* signal condition */
void picoRTOS_cond_broadcast(/*@notnull@*/ struct picoRTOS_cond * cond);                                /* broadcast condition */
void picoRTOS_cond_wait(/*@notnull@*/ struct picoRTOS_cond *cond,
                        /*@notnull@*/ struct picoRTOS_mutex *mutex);                                    /* wait for condition */

#endif
