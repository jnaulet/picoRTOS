#ifndef PICORTOS_PORT_H
#define PICORTOS_PORT_H

#include "picoRTOS_types.h"

/* switch context */
/*@exposed@*/ picoRTOS_stack_t *
picoRTOS_switch_context(/*@notnull@*/ picoRTOS_stack_t *sp);

/* TICK */
/*@exposed@*/ picoRTOS_stack_t *
picoRTOS_tick(/*@notnull@*/ picoRTOS_stack_t *sp);  /* update tick */

/* ARCH : FUNCTIONS TO IMPLEMENT */

extern void arch_init(void);    /* init architecture */
extern void arch_suspend(void); /* suspends tick */
extern void arch_resume(void);  /* resumes tick */

/*@temp@*/ extern picoRTOS_stack_t *
arch_prepare_stack(/*@notnull@*/ struct picoRTOS_task *task);
/*@noreturn@*/ extern void
arch_start_first_task(/*@notnull@*/ picoRTOS_stack_t *sp);

extern void arch_yield(void);
/*@noreturn@*/ extern void arch_idle(/*@null@*/ void *null);

/* ARCH: ATOMIC OPS (optional) */

/*@external@*/ extern picoRTOS_atomic_t
arch_test_and_set(/*@notnull@*/ picoRTOS_atomic_t *ptr); /* atomic test and set */
/*@external@*/ extern picoRTOS_atomic_t
arch_compare_and_swap(/*@notnull@*/ picoRTOS_atomic_t *var,
                      picoRTOS_atomic_t old,
                      picoRTOS_atomic_t val);               /* atomic compare and swap */

/* ARCH: INTERRUPTS (optional) */

/*@external@*/ extern void
arch_register_interrupt(picoRTOS_irq_t irq,
                        /*@notnull@*/ picoRTOS_isr_fn fn,
                        /*@null@*/ void *priv);

/*@external@*/ extern void arch_enable_interrupt(picoRTOS_irq_t irq);
/*@external@*/ extern void arch_disable_interrupt(picoRTOS_irq_t irq);

#endif
