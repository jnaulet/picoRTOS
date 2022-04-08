#include "picoRTOS-SMP.h"
#include "picoRTOS_types.h"
#include "picoRTOS_port.h"

/*@external@*/ extern picoRTOS_stack_t *arch_core_sp;
/*@external@*/ extern picoRTOS_stack_t *arch_task_sp;
/*@external@*/ extern unsigned long *arch_core_ivpr;

/* ASM */
/*@external@*/ extern void arch_core_start(void);

#define MC_ME_GS    ((volatile unsigned long*)0xfffb8000)
#define MC_ME_MCTL  ((volatile unsigned long*)0xfffb8004)
#define MC_ME_CCTL  ((volatile unsigned short*)0xfffb81c6)
#define MC_ME_CADDR ((volatile unsigned long*)0xfffb81e4)

static void wait_for_transition_complete(void)
{
    int n = 0xffff;
    volatile unsigned long reg;

    do
        reg = *MC_ME_GS;
    while ((reg & 0x8000000ul) != 0 &&
           n-- != 0);

    arch_assert(n != -1);
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

    unsigned long mctl = *MC_ME_MCTL & 0xffff0000ul;

    /* in all modes + start */
    MC_ME_CCTL[core] = (unsigned short)0xfe;
    MC_ME_CADDR[core] = (unsigned long)arch_core_start | 0x1;

    /* enable */
    *MC_ME_MCTL = mctl | 0x5af0ul;
    *MC_ME_MCTL = mctl | 0xa50ful;

    /* wait for transition */
    wait_for_transition_complete();
}
