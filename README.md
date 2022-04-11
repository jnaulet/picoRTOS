# picoRTOS
Very small, very limited, lightning fast, yet portable RTOS with SMP suppport

## Presentation
picoRTOS is a teeny tiny RTOS with as little overhead as humanely possible.

It is very hard real time oriented, meaning it doesn't offer support for interrupts
(except for PIT and context switching) and you shouldn't use any other, as they introduce
jitter anyway.

## Supported architectures

 - TI C2000 / C28x (working)
 - ARM Cortex M0+ (working)
 - PowerPC e200 (working)
 - PowerPC e200 SMP (working)
 - POSIX threads / Linux simulator (working)
 - WIN32 threads / Windows simulator (working)

## Limitations
To increase speed and predictability, every task is identified by its exclusive
level of priority, no round robin support is offered (yet ?).

Priorities are reversed, meaning 0 is the maximum priority task.

## Improvements
Some round-robin-ish support might be implemented with no impact on performance
by allowing different tasks to share the same level of priority and sorting
them at init time in picoRTOS_add_task.

## How to use
Copy the picoRTOS directory in your project and add picoRTOS.c to your build.

Create a picoRTOSConfig.h file at the root of your project.
Sample configs are available for every supported cpu in arch///samples

Then, add the relevant arch files to your build.

Example for ARM Cortex-M0+:

    SRC += picoRTOS/picoRTOS.c
    SRC += picoRTOS/arch/arm/cm0+/picoRTOS_port.c
    SRC += picoRTOS/arch/arm/cm0+/picoRTOS_portasm.S
    CFLAGS += -IpicoRTOS -IpicoRTOS/arch/arm/cm0+

PowerPC can be a pain as it is not a self-contained chip. Here is an example:

    SRC += picoRTOS/picoRTOS.c
    SRC += picoRTOS/arch/ppc/e200/picoRTOS_port.c
    SRC += picoRTOS/arch/ppc/e200/picoRTOS_portasm.S
    SRC += picoRTOS/arch/ppc/e200/timer/timer-pit.c
    SRC += picoRTOS/arch/ppc/e200/intc/intc-mpc574xx.c
    CFLAGS += -IpicoRTOS -IpicoRTOS/arch/ppc/e200

Some extra CONFIG_ARCH_PPC_E200_... will have to be set in your picoRTOSConfig.h
file to provide the registers addresses corresponding to your microcontroller.

---

Code-wise, using picoRTOS is quite straightforward :

    #include "picoRTOS.h"
    
    void main(void)
    {
        picoRTOS_init();
    
        struct picoRTOS_task task0;
        struct picoRTOS_task task1;
        static picoRTOS_stack_t stack0[CONFIG_DEFAULT_STACK_COUNT];
        static picoRTOS_stack_t stack1[CONFIG_DEFAULT_STACK_COUNT];
        ...
    
        picoRTOS_task_init(&task0, task0_main, &task0_context, stack0, CONFIG_DEFAULT_TASK_COUNT);
        picoRTOS_task_init(&task1, task1_main, &task1_context, stack1, CONFIG_DEFAULT_TASK_COUNT);
        ...
    
        picoRTOS_add_task(&task0, 0);
        picoRTOS_add_task(&task1, 1);
        ...
    
        picoRTOS_start();
    }

Hint: tasks are converted to internal structures in picoRTOS_add_task and can be local
but stacks need to be persistant (prefer static to globals to reduce scope).

## Port to a new architecture

To port to a new architecture, you just have to implement the ARCH interface you
will find in picoRTOS.h and/or picoRTOS-SMP.h.

Take a look at the arch subdirectory to get an idea.
