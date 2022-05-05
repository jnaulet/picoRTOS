# picoRTOS

Very small, lightning fast, yet portable RTOS with SMP suppport

## Presentation

picoRTOS is a teeny tiny RTOS with as little overhead as humanely possible.

## Supported architectures

### Single core

 - ARM Cortex-M0+
 - ARM Cortex-M3
 - ATmega2560 / AVR6
 - ATmega328P / AVR5
 - C2000 / C28x
 - PowerPC e200

### Multi-core SMP

 - PowerPC e200 SMP
 - RP2040 SMP (Raspberry Pico)

### Simulation

 - POSIX threads / Linux
 - WIN32 threads / Windows

## Working principle

On every new schedule (tick) task 0 is executed first.
Any call to a sleeping function (picoRTOS_schedule, picoRTOS_sleep or
picoRTOS_sleep_until) will allow the scheduler to move to the next task until
it reaches idle or a new tick occurs and the cycle starts over.

To increase speed and predictability, every task is identified by its exclusive
level of priority, no round robin support is offered.

No memory management is offered, everything is static, which makes the static analyzer's
job much easier for critical applications.

## Advanced features

IPCs are available to architectures that support the correct associated atomic operations,
though no emulation is provided through activation/deactivation of the interrupts
for the ones that don't, as this would go against the hard real time philosophy of
the project.

### The following IPCs are provided:

 - spinlocks (require arch_test_and_set)
 - futexes (require arch_test_and_set)
 - re-entrant mutexes (require arch_compare_and_swap)
 - conditions (require mutexes)

## How to use

Copy the picoRTOS directory in your project and add picoRTOS.c to your build.

Create a picoRTOSConfig.h file at the root of your project.
Sample configs are available for every supported cpu in arch/x/y/samples

Then, add the relevant arch files to your build.

Example for ARM Cortex-M0+:

    SRC += picoRTOS/picoRTOS.c
    SRC += picoRTOS/arch/arm/cm0+/picoRTOS_port.c
    SRC += picoRTOS/arch/arm/cm0+/picoRTOS_portasm.S
    CFLAGS += -IpicoRTOS -IpicoRTOS/arch/arm/cm0+

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

## Demo

A bunch of demo code is available under the demo directory so you can see for yourself
if this software suits your needs.
