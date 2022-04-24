#include "picoRTOS.h"
#include "picoRTOS_port.h"

#define E_B 0x78000000ul /* e_b mnemonic */

/* ASM */
/*@external@*/ extern void arch_EE(void);
/*@external@*/ extern void arch_SC(void);
/*@external@*/ extern void arch_TICK(void);
/*@external@*/ extern unsigned long arch_MSR(void);
/*@external@*/ /*@temp@*/ extern unsigned long *arch_IVPR(void);
/*@external@*/ extern void arch_start_first_task(picoRTOS_stack_t *sp);
/*@external@*/ extern picoRTOS_atomic_t arch_test_and_set(picoRTOS_atomic_t *ptr);
/*@external@*/ extern picoRTOS_atomic_t arch_compare_and_swap(picoRTOS_atomic_t *var,
                                                              picoRTOS_atomic_t old,
                                                              picoRTOS_atomic_t val);
/* FUNCTIONS TO IMPLEMENT */

static void timer_init(void)
{
    *PIT_MCR &= ~0x2;       /* enable PIT */
    *PIT_LDVAL3 = (unsigned long)(CONFIG_SYSCLK_HZ / CONFIG_TICK_HZ) - 1ul;
    *PIT_TCTRL3 |= 0x3;     /* enable interrupt & start */
}

static void intc_init(void)
{
    unsigned long *VTBA = (unsigned long*)(*INTC_IACKR & 0xfffff000);

    *INTC_BCR = 0;
    *INTC_CPR = 0;

    /* TICK */
    VTBA[PIT_IRQ] = (unsigned long)arch_TICK;
    /* priority 1 on any core */
    INTC_PSR[PIT_IRQ] = (unsigned short)0xf001;
}

void arch_init(void)
{
    /* disable interrupts */
    ASM("wrteei 0");

    /* INTERRUPTS */
    unsigned long *IVPR = arch_IVPR();

    /* external exception */
    IVPR[0x10] = E_B | ((unsigned long)arch_EE - (unsigned long)&IVPR[0x10]);
    /* syscall */
    IVPR[0x20] = E_B | ((unsigned long)arch_SC - (unsigned long)&IVPR[0x20]);

    /* TIMER */
    timer_init();

    /* INTERRUPT CONTROLLER */
    intc_init();
}

void arch_suspend(void)
{
    /* disable tick */
    ASM("wrteei 0");
}

void arch_resume(void)
{
    /* enable tick */
    ASM("wrteei 1");
}

picoRTOS_stack_t *arch_prepare_stack(struct picoRTOS_task *task)
{
    /* decrementing stack */
    picoRTOS_stack_t *sp = task->stack + task->stack_count;

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
    ASM("se_sc");
}

void arch_idle(void *null)
{
    arch_assert(null == NULL);

    for (;;)
        ASM("wait");
}
