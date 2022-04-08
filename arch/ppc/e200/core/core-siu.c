#include "picoRTOSConfig.h"
#include "picoRTOS_types.h"
#include "picoRTOS-SMP_port.h"

/*@external@*/ extern picoRTOS_stack_t *arch_core_sp;
/*@external@*/ extern picoRTOS_stack_t *arch_task_sp;
/*@external@*/ extern unsigned long *arch_core_ivpr;

#define HLT1    ((unsigned long*)0xc3f909a4)
#define RSTVEC1 ((unsigned long*)0xc3f909b0)

void arch_core_init(picoRTOS_core_t core,
                    picoRTOS_stack_t *stack,
                    picoRTOS_size_t stack_count,
                    picoRTOS_stack_t *sp)
{
    arch_assert(core > 0);
    arch_assert(core < CONFIG_SMP_CORES);
    arch_assert(stack != NULL);
    arch_assert(stack_count >= (picoRTOS_size_t)ARCH_MIN_STACK_COUNT);
    arch_assert(sp != NULL);

    arch_core_sp = stack + stack_count;
    arch_task_sp = sp;
    arch_core_ivpr = arch_IVPR();

    /* start, rst + vle */
    RSTVEC1[core - 1] = (unsigned long)arch_core_start | 0x3;
    *HLT1 &= ~(1u << (31 - core));
}
