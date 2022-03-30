#ifndef PICORTOS_TYPES_H
#define PICORTOS_TYPES_H

/* optimize for speed */
typedef unsigned int picoRTOS_stack_t;
typedef unsigned int picoRTOS_tick_t;
typedef unsigned int picoRTOS_size_t;
typedef unsigned int picoRTOS_priority_t;

#define ARCH_INTIAL_STACK_COUNT 38
#define ARCH_MIN_STACK_COUNT (ARCH_INTIAL_STACK_COUNT + 2)

/* splint cannot check inline assembly */
#ifdef S_SPLINT_S
# define ASM(x) {}
#else
# define ASM(x) asm (x)
#endif

#define arch_assert(x) if (!(x)) ASM("  ESTOP0")

#endif
