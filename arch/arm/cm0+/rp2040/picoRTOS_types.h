#ifndef PICORTOS_TYPES_H
#define PICORTOS_TYPES_H

typedef unsigned long picoRTOS_stack_t;
typedef unsigned long picoRTOS_tick_t;
typedef unsigned long picoRTOS_size_t;
typedef unsigned long picoRTOS_priority_t;
typedef unsigned long picoRTOS_atomic_t;
typedef unsigned long picoRTOS_mask_t;
typedef unsigned long picoRTOS_core_t;

#define ARCH_INITIAL_STACK_COUNT 16
#define ARCH_MIN_STACK_COUNT (ARCH_INITIAL_STACK_COUNT + 8) /* must be 8-bytes "aligned" */
#define ARCH_SMP_MIN_STACK_COUNT 0

/* splint doesn't like inline assembly */
#ifdef S_SPLINT_S
# define ASM(x) {}
#else
# define ASM(x) __asm__ volatile (x)
#endif

#define arch_assert(x) if (!(x)) ASM("bkpt")

#endif
