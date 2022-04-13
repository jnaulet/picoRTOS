#ifndef PICORTOS_H
#define PICORTOS_H

#include <stddef.h>

#include "picoRTOS_types.h"
#include "picoRTOSConfig.h"

/* TASKS */
typedef void (*picoRTOS_task_fn_t)(void*);

struct picoRTOS_task {
    /*@temp@*/ picoRTOS_task_fn_t fn;
    /*@temp@*/ void *priv;
    /*@temp@*/ picoRTOS_stack_t *stack;
    picoRTOS_size_t stack_count;
};

void picoRTOS_task_init(/*@out@*/ struct picoRTOS_task *task,
                        picoRTOS_task_fn_t fn, /*@null@*/ void *priv,
                        /*@notnull@*/ picoRTOS_stack_t *stack,
                        picoRTOS_size_t stack_count);

/* SCHEDULER */
void picoRTOS_init(void);
void picoRTOS_add_task(/*@notnull@*/ struct picoRTOS_task *task,
                       picoRTOS_priority_t prio);               /* register task */

/*@maynotreturn@*/ void picoRTOS_start(void);                   /* starts picoRTOS */
void picoRTOS_suspend(void);                                    /* suspends the scheduling */
void picoRTOS_resume(void);                                     /* resumes the scheduling */

void picoRTOS_schedule(void);                                   /* move to next task */
void picoRTOS_sleep(picoRTOS_tick_t delay);                     /* put current task to sleep */
void picoRTOS_sleep_until(/*@notnull@*/ picoRTOS_tick_t *ref,   /* put current task to sleep until */
                          picoRTOS_tick_t period);
/*@noreturn@*/ void picoRTOS_kill(void);                        /* kills the current task */
picoRTOS_priority_t picoRTOS_self(void);                        /* gets the current thread priority */

/* switch context */
/*@exposed@*/ picoRTOS_stack_t *
picoRTOS_switch_context(/*@notnull@*/ picoRTOS_stack_t *sp);

/* TICK */
/*@exposed@*/ picoRTOS_stack_t *
picoRTOS_tick(/*@notnull@*/ picoRTOS_stack_t *sp);  /* update tick */

picoRTOS_tick_t picoRTOS_get_tick(void);            /* get current tick */

/* TIME */
#define PICORTOS_DELAY_SEC(x) (picoRTOS_tick_t)((x) * CONFIG_TICK_HZ)
#define PICORTOS_DELAY_MSEC(x) (picoRTOS_tick_t)(((x) * CONFIG_TICK_HZ) / 1000)
#define PICORTOS_DELAY_USEC(x) (picoRTOS_tick_t)(((x) * CONFIG_TICK_HZ) / 1000000)

/* ARCH : FUNCTIONS TO IMPLEMENT */

extern void arch_init(void);    /* init architecture */
extern void arch_suspend(void); /* suspends tick */
extern void arch_resume(void);  /* resumes tick */

/*@temp@*/ extern picoRTOS_stack_t *
arch_prepare_stack(/*@notnull@*/ struct picoRTOS_task *task);
/*@noreturn@*/ /*@external@*/ extern void
arch_start_first_task(/*@notnull@*/ picoRTOS_stack_t *sp);

extern void arch_yield(void);
/*@noreturn@*/ extern void arch_idle(/*@null@*/ void *null);

/* assert */
#ifndef arch_assert
# define arch_assert(x) if (!(x)) for (;;)
#endif

#endif
