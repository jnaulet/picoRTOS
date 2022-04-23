# Release notes

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
