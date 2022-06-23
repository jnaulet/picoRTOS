# Release notes

## picoRTOS v1.5
### What's new ?

This version introduces support for contextual interrupt management. A new API is
available :
 - void picoRTOS_register_interrupt(picoRTOS_irq_t irq, picoRTOS_isr_fn fn, void *priv);
 - void picoRTOS_enable_interrupt(picoRTOS_irq_t irq);
 - void picoRTOS_disable_interrupt(picoRTOS_irq_t irq);

The following architectures are supported:
 - PPC e200
 - AVR5
 - AVR6
 - ARM Cortex-M0+
 - ARM Cortex-M3
 - ARM Cortex-M4F

This is still experimental, it is quite hard to test as usually on a hard RTOS you only
need 1 or 2 interrupts (tick and context switch)

## picoRTOS v1.4.4
### What's new ?

Fixed a potential issue with tick overrun in picoRTOS_sleep_until

## picoRTOS v1.4.3
### What's new ?

Fixed a huge bug in ARM Cortex-M3 & M4 compare_and_swap atomic functions.

Moved all functions related to ports in arch/include to avoid them being accidentally
called by an unsuspecting user. Internal mechanics are now more or less hidden to
the user.

Replaced the useless picoRTOS_size_t type by the standard size_t type.

## picoRTOS v1.4.2
### What's new ?

We finally gave up on the policy that totally forbids CPU without native support for atomic 
operations to use IPCs. A new keyword/define (CONFIG_ARCH_EMULATE_ATOMIC) has been added
for these specific targets:
 - Cortex-M0+
 - AVR5 (ATMega328P)
 - AVR6 (ATMega2560)

Activating this option will allow IPCs on these chips and sightly lower their real-time
performance.

picoRTOS_sleep and picoRTOS_sleep_until functions have been protected a little bit better

## picoRTOS v1.4.1
### What's new ?

ARM context switching assembly code has been simplified a lot, removing as many useless
instructions as possible.

A major bug has been fixed on Cortex-M4, FPU was not configured properly and led to
crashes if used.

## picoRTOS v1.4.0
### What's new ?

One new architecture has been added:
 - Cortex-M4F (ARM)

The corresponding blink demo has been added to the demo directory
at demo/s32k142evb-q100 and the static analysis is performed by the
makefile.

## picoRTOS v1.3.2
### What's new ?

On NXP MPC574xC/G, the e200z2 can now be used as an auxiliary core and a
major bug has been fixed (auxilary core crashed when trying to access the data section).
The picoRTOS_port.h file has been removed (for better or for worse).

The ARM CM0+ family (including RP2040) have been refactored and duplicated asm code
has been removed.

The CM3 port has been slightly improved and IPCs have been added to the Arduino Due demo.

On c28x, the assembly code has been refactored to be compatible with the eabi option
(required if you want to use fpu64).

A /*@unused@*/ splint tag has been added to the trylock functions to notify they
may or may not be called externally.

## picoRTOS v1.3.1
### What's new ?

IPC support has been added to RP2040 (+ demo).
Clear guidelines have been added to the CONTRIBUTING.md file.

## picoRTOS v1.3
### What's new ?

One new SMP architecture has been added:
 - RP2040 (Raspberry Pico)

On this architecture, pwm0 acts as the periodic interrupt timer as the
irq is shared by the 2 cores' NVICs and allows their synchronization.

The minimal stack on ARM has been increased by 4 bytes as some crashes occurred
in -Os mode on severely RAM limited targets (like NXP S32k116)

Adding -DNDEBUG in CFLAGS has proven to cause issues with ARM's VTABLE alignment
so the systematic copy of the VTABLE to RAM has been made optional

## picoRTOS v1.2
### What's new ?

API was improved, picoRTOS now provides the following call:
 - picoRTOS_self (returns the calling task its priority/id)

Ports now can implement two new interfaces related to atomic operations:
 - arch_test_and_set
 - arch_compare_and_swap

The following IPCs were added under the ipc/ directory:
 - spinlocks (require arch_test_and_set)
 - futexes (require arch_test_and_set)
 - re-entrant mutexes (require arch_compare_and_swap)
 - conditions (require mutexes)

Mutexes and conditions are very POSIX-lookalike, for some strange reason :)

Three Arduino architectures and their relative demos are provided:
 - ARM Cortex-M3 (Arduino Due)
 - ATmega2560 (Arduino Mega 2560)
 - ATmega328P (Arduino Uno)

Refactoring of PPC targets to make inclusion in existing projects easier

Usual share of bugfixes on existing supported architectures:
 - Better stack management on PPC and Cortex-M0+
 - Bad use of reservation bit on PPC e200

## picoRTOS v1.1
### What's new ?

Two missing targets and their demo code were added:
 - Linux / phtreads simulator
 - Windows simulator

## picoRTOS v1.0
### What's new ?

Everything. picoRTOS finally goes public, free of charge and expurgated of any problematic
intellectual property. To be on the safe side, this is a very stripped down version, huge
improvements might be added along the way as we discover more non-litigous features. 

The following architectures are supported :
 - TI C2000 / c28x (single core)
 - ARM Cortex-M0+ (single core)
 - PowerPC e200 series (single core and multicore SMP)

Some demo code is provided for these 3 architectures under demo/
