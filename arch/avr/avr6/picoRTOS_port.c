#include "picoRTOS.h"

/* ASM */
/*@external@*/ extern void arch_start_first_task(picoRTOS_stack_t *sp);
/*@external@*/ extern void arch_TIMER1_COMPA(void *priv);

/* AVR is one of the rare CPUs that can switch contexts without an interrupt,
 * so this is directly defined in assembly language */
/*@external@*/ extern void arch_yield(void);

/* interrupts (avr-specific, exclude from static analysis) */
#ifndef S_SPLINT_S
# include <avr/interrupt.h>
# define TIMER1_COMPA_IRQ TIMER1_COMPA_vect_num
#else
# define TIMER1_COMPA_IRQ 0
#endif

/* systick registers */
#define AVR_TIMSK1 ((volatile unsigned char*)0x6f)
#define AVR_TCCR1A ((volatile unsigned char*)0x80)
#define AVR_TCCR1B ((volatile unsigned char*)0x81)
#define AVR_TCCR1C ((volatile unsigned char*)0x82)
#define AVR_TCNT1L ((volatile unsigned char*)0x84)
#define AVR_TCNT1H ((volatile unsigned char*)0x85)
#define AVR_OCR1AL ((volatile unsigned char*)0x88)
#define AVR_OCR1AH ((volatile unsigned char*)0x89)

/* SETUP */
static void timer_setup(void)
{
    unsigned int ocr1a = (unsigned int)(CONFIG_SYSCLK_HZ / CONFIG_TICK_HZ) - 1u;

    *AVR_TCCR1A = (unsigned char)0;
    *AVR_TCCR1B = (unsigned char)(1u << 3); /* CTC mode */
    *AVR_TCCR1C = (unsigned char)0;

    /* hi byte is always written first */
    *AVR_TCNT1H = (unsigned char)0;
    *AVR_TCNT1L = (unsigned char)0;

    *AVR_OCR1AH = (unsigned char)(ocr1a >> 8);
    *AVR_OCR1AL = (unsigned char)ocr1a;

    /* output compare A match interrupt enable */
    *AVR_TIMSK1 |= (unsigned char)(1u << 1);
    *AVR_TCCR1B |= (unsigned char)1; /* start, no prescaler */
}

/* FUNCTIONS TO IMPLEMENT */

void arch_init(void)
{
    /* disable interrupts */
    ASM("cli");

    arch_register_interrupt((picoRTOS_irq_t)TIMER1_COMPA_IRQ,
                            arch_TIMER1_COMPA, NULL);

    /* TIMER5 */
    timer_setup();
}

void arch_suspend(void)
{
    /* disable tick */
    ASM("cli");
}

void arch_resume(void)
{
    /* enable tick */
    ASM("sei");
}

picoRTOS_stack_t *arch_prepare_stack(struct picoRTOS_task *task)
{
    /* stack is decrementing */
    picoRTOS_stack_t *sp = task->stack + task->stack_count;

    sp -= ARCH_INTIAL_STACK_COUNT;

    /* avr6 has 22-bit pc */
    sp[35] = (picoRTOS_stack_t)(unsigned int)task->fn;
    sp[34] = (picoRTOS_stack_t)((unsigned int)task->fn >> 8);
    sp[33] = (picoRTOS_stack_t)((unsigned long)task->fn >> 16); /* ret pc */

    sp[30] = (unsigned char)0x0;                                /* r0 */
    sp[29] = (picoRTOS_stack_t)(1u << 7);                       /* sreg (int enable) */

    sp[5] = (picoRTOS_stack_t)(unsigned int)task->priv;         /* r24 */
    sp[4] = (picoRTOS_stack_t)((unsigned int)task->priv >> 8);  /* r25 */

    return sp - 1;                                              /* pop pre-increments */
}

void arch_idle(void *null)
{
    arch_assert(null == NULL);

    for (;;)
        ASM("sleep");
}

/* ATOMIC OPS EMULATION */
#ifdef CONFIG_ARCH_EMULATE_ATOMIC
picoRTOS_atomic_t arch_compare_and_swap(picoRTOS_atomic_t *var,
                                        picoRTOS_atomic_t old,
                                        picoRTOS_atomic_t val)
{
    ASM("cli");

    if (*var == old) {
        *var = val;
        val = old;
    }

    ASM("sei");
    return val;
}

picoRTOS_atomic_t arch_test_and_set(picoRTOS_atomic_t *ptr)
{
    return arch_compare_and_swap(ptr, (picoRTOS_atomic_t)0,
                                 (picoRTOS_atomic_t)1);
}
#endif

/* INTERRUPTS MANAGEMENT */

#define ARCH_VECTOR_COUNT 56

/*@external@*/
extern struct {
    picoRTOS_isr_fn fn;
    /*@temp@*/ /*@null@*/ void *priv;
} ISR_TABLE[ARCH_VECTOR_COUNT];

void arch_register_interrupt(picoRTOS_irq_t irq, picoRTOS_isr_fn fn, void *priv)
{
    ISR_TABLE[irq - 1].fn = fn;
    ISR_TABLE[irq - 1].priv = priv;
}
