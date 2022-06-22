#ifndef PICORTOS_SMP_PORT_H
#define PICORTOS_SMP_PORT_H

#include "picoRTOS_port.h"
#include "picoRTOS_types.h"

/* SMP : ARCH FUNTIONS TO IMPLEMENT */

extern void arch_smp_init(void); /* init picoRTOS SMP for arch */

extern void arch_core_init(picoRTOS_core_t core,
                           /*@notnull@*/ picoRTOS_stack_t *stack,
                           size_t stack_count,
                           /*@notnull@*/ picoRTOS_stack_t *sp);     /* start core */

extern picoRTOS_core_t arch_core(void);                             /* get core id */
extern void arch_spin_lock(void);                                   /* protects code section from other cores */
extern void arch_spin_unlock(void);                                 /* ends code section protection */
extern void arch_memory_barrier(void);                              /* memory barrier (sync) */

/* INTERRUPT MANAGEMENT (optional) */

/*@external@*/
extern void arch_smp_enable_interrupt(picoRTOS_irq_t irq,
                                      picoRTOS_mask_t core_mask);
/*@external@*/
extern void arch_smp_disable_interrupt(picoRTOS_irq_t irq,
                                       picoRTOS_mask_t core_mask);

#endif
