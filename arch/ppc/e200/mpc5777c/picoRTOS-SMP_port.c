#include "picoRTOS-SMP.h"
#include "picoRTOS_port.h"

#ifndef CONFIG_DEADLOCK_COUNT
# error Deadlock count is not defined
#endif

/* regs */
#define SEMA42_BASE 0xfff24000

#define SEMA42_GATE0 ((unsigned char*)SEMA42_BASE)
#define SEMA42_RSTGT ((unsigned short*)(SEMA42_BASE + 0x100))

#define HLT1    ((unsigned long*)0xc3f909a4)
#define RSTVEC1 ((unsigned long*)0xc3f909b0)

/* ASM */
/*@external@*/ extern unsigned long *arch_IVPR(void);
/*@external@*/ extern picoRTOS_core_t arch_core(void);
/*@external@*/ extern void arch_memory_barrier(void);
/*@external@*/ extern void arch_core_start(void);

/*@external@*/ extern picoRTOS_stack_t *arch_core_sp;
/*@external@*/ extern picoRTOS_stack_t *arch_task_sp;
/*@external@*/ extern unsigned long *arch_core_ivpr;

static void smp_intc_init(void)
{
    picoRTOS_size_t n = (picoRTOS_size_t)CONFIG_SMP_CORES;
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

void arch_core_init(picoRTOS_core_t core,
                    picoRTOS_stack_t *stack,
                    picoRTOS_size_t stack_count,
                    picoRTOS_stack_t *sp)
{
    arch_assert(core > 0);
    arch_assert(core < (picoRTOS_core_t)CONFIG_SMP_CORES);
    arch_assert(stack != NULL);
    arch_assert(stack_count >= (picoRTOS_size_t)ARCH_MIN_STACK_COUNT);
    arch_assert(sp != NULL);

    arch_core_sp = stack + (stack_count - 1);
    arch_task_sp = sp;
    arch_core_ivpr = arch_IVPR();

    /* start, rst + vle */
    RSTVEC1[core - 1] = (unsigned long)arch_core_start | 0x3;
    *HLT1 &= ~(1u << (31 - core));
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
