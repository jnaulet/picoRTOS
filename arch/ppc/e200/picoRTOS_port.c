#include "picoRTOS.h"
#include "picoRTOS_port.h"

/* EXCEPTIONS */
/*@external@*/ extern void arch_EE(void);
/*@external@*/ extern void arch_SC(void);

#define E_B 0x78000000ul /* e_b mnemonic */

/* FUNCTIONS TO IMPLEMENT */

void arch_init(void)
{
    /* disable interrupts */
    __asm__ volatile ("wrteei 0");

    /* INTERRUPTS */
    unsigned long *IVPR = arch_IVPR();

    /* external exception */
    IVPR[0x10] = E_B | ((unsigned long)arch_EE - (unsigned long)&IVPR[0x10]);
    /* syscall */
    IVPR[0x20] = E_B | ((unsigned long)arch_SC - (unsigned long)&IVPR[0x20]);

    /* TIMER */
    arch_timer_init();

    /* INTERRUPT CONTROLLER */
    arch_intc_init();
}

void arch_suspend(void)
{
    /* disable tick */
    __asm__ volatile ("wrteei 0");
}

void arch_resume(void)
{
    /* enable tick */
    __asm__ volatile ("wrteei 1");
}

picoRTOS_stack_t *arch_prepare_stack(struct picoRTOS_task *task)
{
    /* decrementing stack */
    picoRTOS_stack_t *sp = task->stack + (task->stack_count - 1);

    arch_assert(task != NULL);

    /* Allocate stack */
    sp -= ARCH_INITIAL_STACK_COUNT;

    /* r[14-31] : 18 registers */
    /* r[0, 3-12] : 11 registers */
    sp[7] = (picoRTOS_stack_t)task->priv;       /* r3 */
    /* xer, xtr, lr, cr : 4 registers */
    sp[5] = 0;                                  /* xer */
    sp[4] = 0;                                  /* ctr */
    sp[3] = 0;                                  /* lr */
    sp[2] = 0;                                  /* cr */
    /* srr[0-1] : 2 registers */
    sp[1] = 0x29000ul | arch_MSR();             /* srr1 */
    sp[0] = (picoRTOS_stack_t)task->fn;         /* srr0 / pc */

    return sp;
}

void arch_yield(void)
{
    __asm__ volatile ("se_sc");
}

void arch_idle(void *null)
{
    arch_assert(null == NULL);

    for (;;)
        __asm__ volatile ("wait");
}
