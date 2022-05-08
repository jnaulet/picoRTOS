#include "picoRTOS-SMP.h"

/* you need Raspberry pico SDK for this target */
#ifndef S_SPLINT_S
# include "pico/multicore.h"
#else
# define multicore_launch_core1(x) x()
#endif

#ifndef CONFIG_DEADLOCK_COUNT
# error Deadlock count is not defined
#endif

#ifndef PWM_BASE
# define PWM_BASE 0x40050000
#endif

/* PWM */
#define PWM_CH0_CSR ((volatile unsigned long*)(PWM_BASE + 0))
#define PWM_CH0_DIV ((volatile unsigned long*)(PWM_BASE + 0x4))
#define PWM_CH0_TOP ((volatile unsigned long*)(PWM_BASE + 0x10))
#define PWM_INTR    ((volatile unsigned long*)(PWM_BASE + 0xa4))
#define PWM_INTE    ((volatile unsigned long*)(PWM_BASE + 0xa8))

/* NVIC */
#define NVIC_ISER         ((volatile unsigned long*)0xe000e100)
#define NVIC_ICPR         ((volatile unsigned long*)0xe000e280)
#define NVIC_SHPR3        ((volatile unsigned long*)0xe000ed20)

/* SIO */
#define SIO_CPUID     ((volatile unsigned long*)(SIO_BASE + 0))
#define SIO_SPINLOCK0 ((volatile unsigned long*)(SIO_BASE + 0x100))

/* VTOR */
#define VTOR ((volatile unsigned long*)0xe000ed08)

#define SYSCLK_DIV 16ul

/* ASM */
/*@external@*/ extern void arch_PWM_WRAP(void);
/*@external@*/ extern void arch_PENDSV(void);
/*@external@*/ extern void arch_start_first_task(picoRTOS_stack_t *sp);

/*@temp@*/ static picoRTOS_stack_t *core1_first_task_sp;

void arch_smp_init(void)
{
    /* RP2040 VTABLE is fixed, can't move it like single-core port does */
    unsigned long *VTABLE = (unsigned long*)*VTOR;

    /* disable interrupts */
    ASM("cpsid i");

    VTABLE[14] = (unsigned long)arch_PENDSV;
    VTABLE[20] = (unsigned long)arch_PWM_WRAP;

    /* enable PWM irq */
    *NVIC_ICPR = (1ul << 4);
    *NVIC_ISER |= (1ul << 4);

    /* set PENDSV to min priority */
    *NVIC_SHPR3 |= 0xffff0000ul;

    /* PWM as system clock */
    unsigned long clk = CONFIG_SYSCLK_HZ / SYSCLK_DIV;

    *PWM_CH0_DIV = (SYSCLK_DIV << 4);
    *PWM_CH0_TOP = (clk / CONFIG_TICK_HZ) - 1ul;

    *PWM_INTR = 0x1ul;      /* clear interrupt */
    *PWM_INTE = 0x1ul;      /* enable interrupt */
    *PWM_CH0_CSR = 0x1ul;   /* enable pwm */
}

static void core1_start_first_task(void)
{
    /* enable PWM irq */
    *NVIC_ICPR = (1ul << 4);
    *NVIC_ISER |= (1ul << 4);

    /* set PENDSV to min priority */
    *NVIC_SHPR3 |= 0xffff0000ul;

    arch_start_first_task(core1_first_task_sp);
}

void arch_core_init(picoRTOS_core_t core,
                    picoRTOS_stack_t *stack,
                    picoRTOS_size_t stack_count,
                    picoRTOS_stack_t *sp)
{
    /* only 1 auxiliary core */
    arch_assert(core == (picoRTOS_core_t)1);

    stack += stack_count;
    core1_first_task_sp = sp;

    multicore_launch_core1(core1_start_first_task);
}

picoRTOS_core_t arch_core(void)
{
    return (picoRTOS_core_t)*SIO_CPUID;
}

void arch_spin_lock(void)
{
    int loop = CONFIG_DEADLOCK_COUNT;

    /* spins full throttle */
    while (*SIO_SPINLOCK0 == 0 && loop-- != 0) {
    }

    /* potential deadlock */
    arch_assert(loop != -1);
}

void arch_spin_unlock(void)
{
    *SIO_SPINLOCK0 = 1ul;
}

void arch_memory_barrier(void)
{
    ASM("dmb ish");
}


