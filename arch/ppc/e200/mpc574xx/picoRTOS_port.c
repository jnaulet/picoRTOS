#include "picoRTOS.h"
#include "picoRTOS_port.h"

#define INTC_BASE   0xfc040000
#define PIT_BASE    0xfff84000
#define PIT_IRQ     229

#define INTC_BCR   ((volatile unsigned long*)INTC_BASE)
#define INTC_CPR   ((volatile unsigned long*)(INTC_BASE + 0x10))
#define INTC_IACKR ((volatile unsigned long*)(INTC_BASE + 0x20))
#define INTC_EOIR  ((volatile unsigned long*)(INTC_BASE + 0x30))
#define INTC_PSR   ((volatile unsigned short*)(INTC_BASE + 0x60))

#define PIT_MCR     ((volatile unsigned long*)PIT_BASE)
/* channel 3 */
#define PIT_LDVAL3  ((volatile unsigned long*)(PIT_BASE + 0x130))
#define PIT_CVAL3   ((volatile unsigned long*)(PIT_BASE + 0x134))
#define PIT_TCTRL3  ((volatile unsigned long*)(PIT_BASE + 0x138))
#define PIT_TFLG3   ((volatile unsigned long*)(PIT_BASE + 0x13c))

/* ASM */
/*@external@*/ extern void arch_TICK(void *priv);
/*@external@*/ extern unsigned long arch_MSR(void);
/*@external@*/ extern void arch_start_first_task(picoRTOS_stack_t *sp);
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

void arch_init(void)
{
    /* disable interrupts */
    ASM("wrteei 0");

    /* INTERRUPTS are statically managed in picoRTOS_common.S */

    /* INTERRUPT CONTROLLER */
    *INTC_BCR = 0;
    *INTC_CPR = 0;

    /* TIMER */
    arch_register_interrupt((picoRTOS_irq_t)PIT_IRQ, arch_TICK, NULL);
    arch_enable_interrupt((picoRTOS_irq_t)PIT_IRQ);

    timer_init();
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

/* ATOMIC */

picoRTOS_atomic_t arch_test_and_set(picoRTOS_atomic_t *ptr)
{
    return arch_compare_and_swap(ptr, 0, (picoRTOS_atomic_t)1);
}

/* INTERRUPTS */

/*@external@*/
extern unsigned long arch_EE_private[ARCH_IRQ_COUNT];

void arch_register_interrupt(picoRTOS_irq_t irq, picoRTOS_isr_fn fn, void *priv)
{
    arch_assert(irq < (picoRTOS_irq_t)ARCH_IRQ_COUNT);

    unsigned long *VTBA = (unsigned long*)(*INTC_IACKR & 0xfffff000);

    /* IRQ */
    VTBA[irq] = (unsigned long)fn;
    arch_EE_private[irq] = (unsigned long)priv;
}

void arch_enable_interrupt(picoRTOS_irq_t irq)
{
    arch_assert(irq < (picoRTOS_irq_t)ARCH_IRQ_COUNT);

    /* priority 1 on any core */
    INTC_PSR[irq] = (unsigned short)0xf001;
}

void arch_disable_interrupt(picoRTOS_irq_t irq)
{
    arch_assert(irq < (picoRTOS_irq_t)ARCH_IRQ_COUNT);

    /* priority 0 on any core */
    INTC_PSR[irq] = (unsigned short)0xf000;
}
