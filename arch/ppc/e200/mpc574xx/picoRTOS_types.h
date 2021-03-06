#ifndef PICORTOS_TYPES_H
#define PICORTOS_TYPES_H

#include "picoRTOSConfig.h"

typedef unsigned long picoRTOS_stack_t;
typedef unsigned long picoRTOS_tick_t;
typedef unsigned long picoRTOS_priority_t;
typedef unsigned short picoRTOS_atomic_t;

#define ARCH_INITIAL_STACK_COUNT 36
#define ARCH_MIN_STACK_COUNT (ARCH_INITIAL_STACK_COUNT + 6)

/* SMP */
typedef unsigned long picoRTOS_mask_t;
typedef unsigned long picoRTOS_core_t;

#define ARCH_SMP_MIN_STACK_COUNT 128

/* splint cannot check inline assembly */
#ifdef S_SPLINT_S
# define ASM(x) {}
#else
# define ASM(x) __asm__ volatile (x)
#endif

#define arch_assert(x) if (!(x)) ASM("se_illegal \n\t se_nop")

#endif
