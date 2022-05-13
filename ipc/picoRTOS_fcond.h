#ifndef PICORTOS_FCOND_H
#define PICORTOS_FCOND_H

/* Same as picoRTOS_cond, but for futexes */

#include "picoRTOS.h"
#include "picoRTOS_futex.h"

enum picoRTOS_fcond_act {
    PICORTOS_FCOND_NONE,
    PICORTOS_FCOND_SIGNAL,
    PICORTOS_FCOND_BROADCAST
};

struct picoRTOS_fcond {
    enum picoRTOS_fcond_act act;
    picoRTOS_size_t count;
};

/* macro for static init */
#define PICORTOS_FCOND_INITIALIZER               \
    { PICORTOS_FCOND_NONE, (picoRTOS_size_t)0 }

void picoRTOS_fcond_init(/*@notnull@*/ struct picoRTOS_fcond *fcond);

void picoRTOS_fcond_signal(/*@notnull@*/ struct picoRTOS_fcond *fcond);                             /* signal fcondition */
void picoRTOS_fcond_broadcast(/*@notnull@*/ struct picoRTOS_fcond *fcond);                          /* broadcast fcondition */
void picoRTOS_fcond_wait(/*@notnull@*/ struct picoRTOS_fcond *fcond,
                         /*@notnull@*/ picoRTOS_futex_t *futex);                                    /* wait for fcondition */

#endif
