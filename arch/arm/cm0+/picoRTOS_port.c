#include "picoRTOS.h"

/* SYSTICK */
#define SYSTICK_CSR ((volatile unsigned long*)0xe000e010)
#define SYSTICK_RVR ((volatile unsigned long*)0xe000e014)
#define SYSTICK_CVR ((volatile unsigned long*)0xe000e018)

/* NVIC */
#define NVIC_ICSR         ((volatile unsigned long*)0xe000ed04)
#define NVIC_SHPR3        ((volatile unsigned long*)0xe000ed20)

/* VTOR */
#define VTOR ((volatile unsigned long*)0xe000ed08)

/* ASM */
/*@external@*/ extern void arch_SYSTICK(void);
/*@external@*/ extern void arch_PENDSV(void);
/*@external@*/ extern void arch_start_first_task(picoRTOS_stack_t *sp);
/* no support for atomic ops on this architecture */

/*
 * GCC doesn't align this to 128 when -DNDEBUG flag is passed
 * for some reason, leading to some unexpected crashes so we
 * want to avoid systematic copy, especially if VTABLE is already
 * in RAM
 */
#ifdef CONFIG_ARCH_ARM_MOVE_VTABLE_TO_RAM
/* vector table */
#define VTABLE_COUNT 48
static unsigned long VTABLE[VTABLE_COUNT] __attribute__((aligned(128)));

static void move_vtable_to_ram(void)
{
    int n = VTABLE_COUNT;
    unsigned long *old_VTABLE = (unsigned long*)*VTOR;

    while (n-- != 0)
        VTABLE[n] = old_VTABLE[n];

    *VTOR = (unsigned long)VTABLE;
}
#endif

/* FUNCTIONS TO IMPLEMENT */

void arch_init(void)
{
    /* disable interrupts */
    ASM("cpsid i");

    /* INTERRUPTS */
#ifdef CONFIG_ARCH_ARM_MOVE_VTABLE_TO_RAM
    move_vtable_to_ram();
#else
    unsigned long *VTABLE = (unsigned long*)*VTOR;
#endif
    VTABLE[14] = (unsigned long)arch_PENDSV;
    VTABLE[15] = (unsigned long)arch_SYSTICK;

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
    ASM("cpsid i");
}

void arch_resume(void)
{
    /* enable interrupts */
    ASM("cpsie i");
}

picoRTOS_stack_t *arch_prepare_stack(struct picoRTOS_task *task)
{
    /* ARMs have a decrementing stack */
    picoRTOS_stack_t *sp = task->stack + task->stack_count;

    /* ARM v6 reference manual, section B1.5.6 */
    sp -= ARCH_INITIAL_STACK_COUNT;

    sp[15] = (picoRTOS_stack_t)0x1000000;       /* xpsr */
    sp[14] = (picoRTOS_stack_t)task->fn;        /* return address */
    sp[13] = (picoRTOS_stack_t)picoRTOS_start;  /* lr (r14) */
    /* sp[12-9] = r12, r3, r2, r1 and */
    sp[8] = (picoRTOS_stack_t)task->priv;       /* r0 */
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
        ASM("wfe");
}

/* ATOMIC OPS EMULATION */
#ifdef CONFIG_ARCH_EMULATE_ATOMIC
picoRTOS_atomic_t arch_compare_and_swap(picoRTOS_atomic_t *var,
                                        picoRTOS_atomic_t old,
                                        picoRTOS_atomic_t val)
{
    ASM("cpsid i");

    if (*var == old) {
        *var = val;
        val = old;
    }

    ASM("cpsie i");
    return val;
}

picoRTOS_atomic_t arch_test_and_set(picoRTOS_atomic_t *ptr)
{
    return arch_compare_and_swap(ptr, 0, (picoRTOS_atomic_t)1);
}
#endif
