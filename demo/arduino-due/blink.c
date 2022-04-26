#include "picoRTOS.h"
#include <stdbool.h>

#define BLINK_PERIOD PICORTOS_DELAY_SEC(1)
#define BLINK_DELAY  PICORTOS_DELAY_MSEC(30)

#define BUILTIN_LED (1u << 27)  /* PB27 */
#define TICK        (1u << 18)  /* PA18 */

#define PIOA_PER ((unsigned long*)0x400e0e00)
#define PIOA_OER ((unsigned long*)0x400e0e10)
#define PIOA_SODR ((unsigned long*)0x400e0e30)
#define PIOA_CODR ((unsigned long*)0x400e0e34)

#define PIOB_PER ((unsigned long*)0x400e1000)
#define PIOB_OER ((unsigned long*)0x400e1010)
#define PIOB_SODR ((unsigned long*)0x400e1030)
#define PIOB_CODR ((unsigned long*)0x400e1034)

#define CKGR_MOR ((unsigned long*)0x400e0620)
#define PMC_SR ((unsigned long*)0x400e0668)

static void hw_init(void)
{
    /* PIOA */
    *PIOA_PER |= TICK;
    *PIOA_OER |= TICK;

    /* PIOB */
    *PIOB_PER |= BUILTIN_LED;
    *PIOB_OER |= BUILTIN_LED;

    /* main clock */
    *CKGR_MOR = 0x370028ul; /* 12Mhz internal */
    while ((*PMC_SR & 0x1) != 0) {
        ASM("nop");         /* Warning: can loop indefinitely */
    }
}

static void deepcall_schedule(unsigned long n)
{
    if (n != 0)
        deepcall_schedule(n - 1);
    else
        picoRTOS_schedule();
}

static void tick_main(void *priv)
{
    arch_assert(priv == NULL);

    bool x = false;

    for (;;) {
        /* toggle pin */
        if (x) *PIOA_SODR |= TICK;
        else *PIOA_CODR |= TICK;

        /* stack test */
        deepcall_schedule(20ul);
        x = !x;
    }
}

static void blink_main(void *priv)
{
    picoRTOS_tick_t ref = picoRTOS_get_tick();

    arch_assert(priv == NULL);

    for (;;) {
        picoRTOS_sleep_until(&ref, BLINK_PERIOD);

        /* blink */
        *PIOB_SODR |= BUILTIN_LED;
        picoRTOS_sleep(BLINK_DELAY);
        *PIOB_CODR |= BUILTIN_LED;
        picoRTOS_sleep(BLINK_DELAY);

        *PIOB_SODR |= BUILTIN_LED;
        picoRTOS_sleep(BLINK_DELAY);
        picoRTOS_sleep(BLINK_DELAY);
        *PIOB_CODR |= BUILTIN_LED;
    }
}

static void atomic_test(void)
{
    picoRTOS_atomic_t tset = 0;
    picoRTOS_atomic_t one = (picoRTOS_atomic_t)1;
    picoRTOS_atomic_t two = (picoRTOS_atomic_t)2;

    arch_assert(arch_test_and_set(&tset) == 0);
    arch_assert(arch_test_and_set(&tset) != 0);

    arch_assert(arch_compare_and_swap(&tset, one, two) == one);
    arch_assert(arch_compare_and_swap(&tset, one, two) != one);
}

int main( void )
{
    hw_init();
    picoRTOS_init();

    atomic_test();

    struct picoRTOS_task task;
    static picoRTOS_stack_t stack0[CONFIG_DEFAULT_STACK_COUNT];
    static picoRTOS_stack_t stack1[CONFIG_DEFAULT_STACK_COUNT];

    picoRTOS_task_init(&task, tick_main, NULL, stack0, (picoRTOS_size_t)CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, (picoRTOS_priority_t)TASK_TICK_PRIO);
    picoRTOS_task_init(&task, blink_main, NULL, stack1, (picoRTOS_size_t)CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, (picoRTOS_priority_t)TASK_BLINK_PRIO);

    /* Start the scheduler. */
    picoRTOS_start();

    /* we're not supposed to end here */
    arch_assert(false);
    return -1;
}
