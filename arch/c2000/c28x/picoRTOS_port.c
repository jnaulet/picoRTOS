#include "picoRTOS.h"

/* CPUTIMER2 */
#define CPUTIMER2_TIM ((volatile unsigned long*)0xc10)
#define CPUTIMER2_PRD ((volatile unsigned long*)0xc12)
#define CPUTIMER2_TCR ((volatile unsigned int*)0xc14)
#define CPUTIMER2_TPR ((volatile unsigned long*)0xc16)

/* PIE */
#define PIECTRL      ((volatile unsigned int*)0xce0)
#define PIEVECTTABLE ((volatile unsigned long*)0xd00)

/* INTERRUPTS */
#define INT_CPUTIMER2 0xe
#define INT_RTOS      0x10

/* ASM */
/*@external@*/ extern void arch_INT14(void);
/*@external@*/ extern void arch_RTOSINT(void);

/* SETUP */
static void cputimer2_setup(void)
{
    *CPUTIMER2_PRD = (unsigned long)(CONFIG_SYSCLK_HZ / CONFIG_TICK_HZ);
    *CPUTIMER2_TPR = 0;         /* pre-scale div by 1 */
    *CPUTIMER2_TCR |= 0x30u;    /* stop + reload */
    *CPUTIMER2_TCR &= ~0xc000u; /* hard stop */

    /* register and enable interrupt */
    ASM("  eallow");
    PIEVECTTABLE[INT_CPUTIMER2] = (unsigned long)arch_INT14;
    ASM("  or IER, #0x2000");
    ASM("  edis");

    /* int enable + start */
    *CPUTIMER2_TCR |= 0x4000u;
    *CPUTIMER2_TCR &= ~0x10u;
}

static void rtosint_setup(void)
{
    /* register and enable interrupt */
    ASM("  eallow");
    PIEVECTTABLE[INT_RTOS] = (unsigned long)arch_RTOSINT;
    ASM("  or IER, #0x8000");
    ASM("  edis");
}

/* FUNCTIONS TO IMPLEMENT */

void arch_init(void)
{
    /* disable interrupts */
    ASM(" setc INTM");

    /* CPUTIMER2 */
    cputimer2_setup();
    /* RTOSINT */
    rtosint_setup();

    /* enable interrupt controller */
    *PIECTRL |= 1u;
}

void arch_suspend(void)
{
    /* disable tick */
    ASM("  and IER, #0xdfff");
}

void arch_resume(void)
{
    /* enable tick */
    ASM("  or IER, #0x2000");
}

picoRTOS_stack_t *arch_prepare_stack(struct picoRTOS_task *task)
{
    picoRTOS_size_t i = 0;
    picoRTOS_stack_t *sp = task->stack;

    arch_assert(task != NULL);

    /* End return call stack (debug) */
    sp[i++] = 0;
    sp[i++] = 0;

    /* iret auto save/restore */

    sp[i++] = (picoRTOS_stack_t)0x80;                               /* st0 */
    sp[i++] = 0x0;                                                  /* t */
    sp[i++] = 0x0;                                                  /* acc */
    sp[i++] = 0x0;
    sp[i++] = (picoRTOS_stack_t)0xffff;                             /* p */
    sp[i++] = (picoRTOS_stack_t)0xffff;
    sp[i++] = (picoRTOS_stack_t)0xffff;                             /* ar0 */
    sp[i++] = (picoRTOS_stack_t)0xffff;                             /* ar1 */
    sp[i++] = (picoRTOS_stack_t)0x8a08;                             /* st1 */
    sp[i++] = 0x0;                                                  /* dp */
    sp[i++] = (picoRTOS_stack_t)0xa000;                             /* ier */
    sp[i++] = 0x0;                                                  /* dbgstat */
    sp[i++] = (picoRTOS_stack_t)(unsigned long)task->fn;            /* pc */
    sp[i++] = (picoRTOS_stack_t)((unsigned long)task->fn >> 16);

    sp[i++] = (picoRTOS_stack_t)0xaaaa;     /* interrupt auto dis-alignment */
    sp[i++] = (picoRTOS_stack_t)0xbbbb;     /* context align, set st1 ASP bit (0x10) */

    /* stuff to remember */

    sp[i++] = (picoRTOS_stack_t)0x8a18;                     /* st1 */
    sp[i++] = 0x0;                                          /* dp */
    sp[i++] = 0x0;                                          /* rpc */
    sp[i++] = 0x0;
    sp[i++] = 0x0;                                          /* xt */
    sp[i++] = 0x0;
    sp[i++] = 0x0;                                          /* ar0h */
    sp[i++] = 0x0;                                          /* ar1h */
    sp[i++] = 0x0;                                          /* xar2 */
    sp[i++] = 0x0;
    sp[i++] = 0x0;                                          /* xar3 */
    sp[i++] = 0x0;
    sp[i++] = (picoRTOS_stack_t)(unsigned long)task->priv;  /* xar4 */
    sp[i++] = (picoRTOS_stack_t)((unsigned long)task->priv >> 16);
    sp[i++] = 0x0;                                          /* xar5 */
    sp[i++] = 0x0;
    sp[i++] = 0x0;                                          /* xar6 */
    sp[i++] = 0x0;
    sp[i++] = 0x0;                                          /* xar7 */
    sp[i++] = 0x0;

    return sp + i;
}

void arch_yield(void)
{
    /* RTOSINT interrupt */
    ASM("  or IFR, #0x8000");
}

void arch_idle(void *null)
{
    arch_assert(null == NULL);

    for (;;)
        ASM("  idle");
}
