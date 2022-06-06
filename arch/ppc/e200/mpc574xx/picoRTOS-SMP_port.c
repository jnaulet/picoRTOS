#include "picoRTOS-SMP.h"
#include "picoRTOS-SMP_port.h"

#ifndef CONFIG_DEADLOCK_COUNT
# error Deadlock count is not defined
#endif

/* regs */
#define INTC_BASE   0xfc040000
#define SEMA42_BASE 0xfc03c000
#define MC_ME_BASE  0xfffb8000
#define SIUL2_BASE  0xfffc0000

#define INTC_CPR   ((volatile unsigned long*)(INTC_BASE + 0x10))
#define INTC_IACKR ((volatile unsigned long*)(INTC_BASE + 0x20))
#define INTC_PSR   ((volatile unsigned short*)(INTC_BASE + 0x60))

#define SEMA42_GATE0 ((volatile unsigned char*)SEMA42_BASE)
#define SEMA42_RSTGT ((volatile unsigned short*)(SEMA42_BASE + 0x40))

#define MC_ME_GS    ((volatile unsigned long*)MC_ME_BASE)
#define MC_ME_MCTL  ((volatile unsigned long*)(MC_ME_BASE + 0x4))
#define MC_ME_CCTL  ((volatile unsigned short*)(MC_ME_BASE + 0x1c6))
#define MC_ME_CADDR ((volatile unsigned long*)(MC_ME_BASE + 0x1e4))

#define SIUL2_MIDR2 ((volatile unsigned long*)(SIUL2_BASE + 0x8))

/* ASM */
/*@external@*/ /*@temp@*/ extern unsigned long *arch_IVPR(void);
/*@external@*/ extern unsigned long arch_R13(void);
/*@external@*/ extern picoRTOS_core_t arch_core(void);
/*@external@*/ extern void arch_memory_barrier(void);
/*@external@*/ extern void arch_core_start(void);

/*@external@*/ extern picoRTOS_stack_t *arch_core_sp;
/*@external@*/ extern picoRTOS_stack_t *arch_task_sp;
/*@external@*/ extern unsigned long *arch_core_ivpr;
/*@external@*/ extern unsigned long arch_core_r13;

static void smp_intc_init(void)
{
    size_t n = (size_t)CONFIG_SMP_CORES;
    unsigned long *VTBA = (unsigned long*)(*INTC_IACKR & 0xfffff000);

    while (n-- != 0) {
        INTC_CPR[n] = 0;
        INTC_IACKR[n] = (unsigned long)VTBA;
    }
}

void arch_smp_init(void)
{
    /* single-core */
    arch_init();

    /* reset spinlock gate0 */
    *SEMA42_RSTGT = (unsigned short)0xe200;
    *SEMA42_RSTGT = (unsigned short)0x1d00;

    /* init intc */
    smp_intc_init();
}

static size_t mc_me_index(picoRTOS_core_t core)
{
    char partnum = (char)(*SIUL2_MIDR2 >> 8);

    arch_assert(partnum == 'C' || partnum == 'G');

    if (partnum == 'C') {
        /* C series (2 cores) */
        arch_assert(core == (picoRTOS_core_t)1);
        return (size_t)2;
    }

    if (partnum == 'G') {
        /* G series (3 cores) */
        arch_assert(core < (picoRTOS_core_t)3);
        return (size_t)core;
    }

    /* other series (unsupported) */
    return (size_t)0xfffffff;
}

static void wait_for_transition_complete(void)
{
    int n = CONFIG_DEADLOCK_COUNT;
    volatile unsigned long reg;

    do
        reg = *MC_ME_GS;
    while ((reg & 0x8000000ul) != 0 &&
           n-- != 0);

    arch_assert(n != -1);
}

void arch_core_init(picoRTOS_core_t core,
                    picoRTOS_stack_t *stack,
                    size_t stack_count,
                    picoRTOS_stack_t *sp)
{
    arch_assert(core > 0);
    arch_assert(core < (picoRTOS_core_t)CONFIG_SMP_CORES);
    arch_assert(stack != NULL);
    arch_assert(stack_count >= (size_t)ARCH_MIN_STACK_COUNT);
    arch_assert(sp != NULL);

    arch_core_sp = stack + (stack_count - 1);
    arch_task_sp = sp;
    arch_core_ivpr = arch_IVPR();
    arch_core_r13 = arch_R13();

    size_t index = mc_me_index(core);
    unsigned long mctl = *MC_ME_MCTL & 0xffff0000ul;

    /* in all modes + start */
    MC_ME_CCTL[index] = (unsigned short)0xfe;
    MC_ME_CADDR[index] = (unsigned long)arch_core_start | 0x1;

    /* enable */
    *MC_ME_MCTL = mctl | 0x5af0ul;
    *MC_ME_MCTL = mctl | 0xa50ful;

    /* wait for transition */
    wait_for_transition_complete();
}

void arch_spin_lock(void)
{
    int n = CONFIG_DEADLOCK_COUNT;
    picoRTOS_core_t core = arch_core() + 1;

    do
        *SEMA42_GATE0 = (unsigned char)core;
    while (*SEMA42_GATE0 != (unsigned char)core &&
           n-- != 0);

    /* potential deadlock */
    arch_assert(n != -1);
}

void arch_spin_unlock(void)
{
    *SEMA42_GATE0 = (unsigned char)0;
}

/* INTERRUPTS */

void arch_smp_enable_interrupt(picoRTOS_irq_t irq,
                               picoRTOS_mask_t core_mask)
{
    arch_assert(irq < (picoRTOS_irq_t)ARCH_IRQ_COUNT);

    unsigned short psr = INTC_PSR[irq];

    psr |= (((unsigned short)0x1 & core_mask) << 15);
    psr |= (((unsigned short)0x2 & core_mask) << 13);

    /* priority 1 on any core */
    INTC_PSR[irq] = psr | (unsigned short)0x1;
}

extern void arch_smp_disable_interrupt(picoRTOS_irq_t irq,
                                       picoRTOS_mask_t core_mask)
{
    arch_assert(irq < (picoRTOS_irq_t)ARCH_IRQ_COUNT);

    unsigned short psr = INTC_PSR[irq];

    psr &= ~(((unsigned short)0x1 & core_mask) << 15);
    psr &= ~(((unsigned short)0x2 & core_mask) << 13);

    /* priority 0 on any core */
    INTC_PSR[irq] = psr | (unsigned short)0x1;
}
