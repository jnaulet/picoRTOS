#ifndef PICORTOS_SMP_H
#define PICORTOS_SMP_H

/* SMP extensions for picoRTOS */

#define PICORTOS_SMP

#include "picoRTOS.h"
#include "picoRTOSConfig.h"

/* CORES */

#ifndef CONFIG_SMP_CORES
# define CONFIG_SMP_CORES 1
#endif

#define PICORTOS_SMP_CORE_ANY                           \
    (picoRTOS_mask_t)((1u << CONFIG_SMP_CORES) - 1u)

/* TASKS */

void picoRTOS_SMP_set_core_mask(picoRTOS_priority_t prio,
                                picoRTOS_mask_t core_mask);

/* SMP : ARCH FUNTIONS TO IMPLEMENT */

extern void arch_smp_init(void); /* init picoRTOS SMP for arch */

extern void arch_core_init(picoRTOS_core_t core,
                           /*@notnull@*/ picoRTOS_stack_t *stack,
                           picoRTOS_size_t stack_count,
                           /*@notnull@*/ picoRTOS_stack_t *sp);     /* start core */

extern picoRTOS_core_t arch_core(void);                             /* get core id */
extern void arch_spin_lock(void);                                   /* protects code section from other cores */
extern void arch_spin_unlock(void);                                 /* ends code section protection */
extern void arch_memory_barrier(void);                              /* memory barrier (sync) */

#endif
