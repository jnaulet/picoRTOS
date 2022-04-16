#include "picoRTOS.h"
#include "ipc/picoRTOS_futex.h"
#include <stdbool.h>
#include <stdint.h>

#define WDCR      ((volatile unsigned int*)0x7029)
#define GPBDIR    ((volatile unsigned long*)0x6f9a)
#define GPBSET    ((volatile unsigned long*)0x6fca)
#define GPBCLEAR  ((volatile unsigned long*)0x6fcc)
#define GPBTOGGLE ((volatile unsigned long*)0x6fce)

#define BLINK_PERIOD PICORTOS_DELAY_SEC(1)
#define BLINK_DELAY  PICORTOS_DELAY_MSEC(15)

#define LED_RED   (1ul << 7)
#define LED_BLUE  (1ul << 2)
#define TICK      (1ul << 0)

static void hw_init(void)
{
    ASM("  eallow");
    /* WDOG */
    *WDCR = 0x68;
    /* GPIO */
    *GPBDIR |= TICK;
    *GPBDIR |= LED_RED;
    *GPBDIR |= LED_BLUE;
    ASM(" edis");
}

static void deepcall_schedule(unsigned long n)
{
    if (n != 0)
        deepcall_schedule(n - 1);
    else
        picoRTOS_schedule();
}

static void gpbtoggle(unsigned long mask)
{
    ASM("  eallow");
    *GPBTOGGLE |= mask;
    ASM("  edis");
}

static void gpbset(unsigned long mask)
{
    ASM("  eallow");
    *GPBSET |= mask;
    ASM("  edis");
}

static void gpbclear(unsigned long mask)
{
    ASM("  eallow");
    *GPBCLEAR |= mask;
    ASM("  edis");
}

static void tick_main(void *priv)
{
    arch_assert(priv == NULL);

    for (;;) {

        /* toggle pin */
        gpbtoggle(TICK);

        /* stack test */
        deepcall_schedule(20);
    }
}

static void blink_main(void *priv)
{
    picoRTOS_futex_t futex = 0;
    picoRTOS_tick_t ref = picoRTOS_get_tick();

    arch_assert(priv == NULL);

    for (;;) {
        picoRTOS_sleep_until(&ref, BLINK_PERIOD);

        /* blink */
        gpbclear(LED_BLUE);
        picoRTOS_sleep(BLINK_DELAY);
        gpbset(LED_BLUE);
        gpbclear(LED_RED);
        picoRTOS_sleep(BLINK_DELAY);
        gpbset(LED_RED);

        /* stack test */
        deepcall_schedule(10);

        /* futex test */
        arch_assert(picoRTOS_futex_trylock(&futex) == 0);
        arch_assert(picoRTOS_futex_trylock(&futex) != 0);
        picoRTOS_futex_unlock(&futex);
    }
}

int main( void )
{
    hw_init();
    picoRTOS_init();

    struct picoRTOS_task task;
    static picoRTOS_stack_t stack0[CONFIG_DEFAULT_STACK_COUNT];
    static picoRTOS_stack_t stack1[CONFIG_DEFAULT_STACK_COUNT];

    picoRTOS_task_init(&task, tick_main, NULL, stack0, CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, TASK_TICK_PRIO);
    picoRTOS_task_init(&task, blink_main, NULL, stack1, CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, TASK_BLINK_PRIO);

    /* Start the scheduler. */
    picoRTOS_start();

    /* we're not supposed to end here */
    arch_assert(false);
    return -1;
}
