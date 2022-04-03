#include "picoRTOS.h"

/* SYSTICK */
#define SYSTICK_CSR ((volatile unsigned long*)0xe000e010)
#define SYSTICK_RVR ((volatile unsigned long*)0xe000e014)
#define SYSTICK_CVR ((volatile unsigned long*)0xe000e018)

/* NVIC */
#define NVIC_ICSR         ((volatile unsigned long*)0xe000ed04)
#define NVIC_SHPR3        ((volatile unsigned long*)0xe000ed20)
#define NVIC_INTVECTTABLE ((volatile unsigned long*)0x20000000)

/* ASM */
/*@external@*/ extern void arch_SYSTICK(void);
/*@external@*/ extern void arch_PENDSV(void);

/* FUNCTIONS TO IMPLEMENT */

void arch_init(void)
{
    /* disable interrupts */
    __asm__ volatile ("cpsid i" : : : "memory");

    /* INTERRUPTS */
    NVIC_INTVECTTABLE[14] = (unsigned long)arch_PENDSV;
    NVIC_INTVECTTABLE[15] = (unsigned long)arch_SYSTICK;
    /* set SYSTICK & PENDSV to min priority */
    *NVIC_SHPR3 |= 0xffff0000ul;

    /* SYSTICK */
    *SYSTICK_CSR &= ~1ul;                                       /* stop systick */
    *SYSTICK_CVR = 0ul;                                         /* reset */
    *SYSTICK_RVR = (CONFIG_SYSCLK_HZ / CONFIG_TICK_HZ) - 1ul;   /* set period */
    *SYSTICK_CSR |= 0x7ul;                                      /* enable interrupt & start */
}

void arch_suspend(void)
{
    /* disable interrupts */
    __asm__ volatile ("cpsid i" : : : "memory");
}

void arch_resume(void)
{
    /* enable interrupts */
    __asm__ volatile ("cpsie i" : : : "memory");
}

picoRTOS_stack_t *arch_prepare_stack(struct picoRTOS_task *task)
{
    /* ARMs have a decrementing stack */
    picoRTOS_stack_t *sp = task->stack + (task->stack_count - 1);

    /* ARM v6 reference manual, section B1.5.6 */
    sp -= ARCH_INITIAL_STACK_COUNT;

    sp[15] = (picoRTOS_stack_t)0x1000000;       /* xspr */
    sp[14] = (picoRTOS_stack_t)task->fn;        /* pc */
    sp[13] = (picoRTOS_stack_t)picoRTOS_start;  /* lr */
    /* sp[12-9] = r12, r3, r2 and r1 */
    sp[8] = (picoRTOS_stack_t)task->priv;
    /* sp[7-0] = r11..r4 */

    return sp;
}

void arch_yield(void)
{
    *NVIC_ICSR = (1ul << 28);
}

void arch_idle(void *null)
{
    arch_assert(null == NULL);

    for (;;)
        __asm__ volatile ("wfe");
}
