#include "picoRTOS.h"
#include <stdbool.h>

#include "device_registers.h"

#define LED0 4          /* PA[4] */
#define LED1 0          /* PA[0] */
#define LED2 (144 + 4)  /* PJ[4] */
#define LED3 (112 + 5)  /* PH[5] */
#define LED4 (32 + 4)   /* PC[4] */
#define LED5 (112 + 13) /* PH[13] */
#define LED6 7          /* PA[7] */
#define LED7 10         /* PA[10] */

#define TICK 144        /* PJ[0] */

#define LED_DELAY_SHORT PICORTOS_DELAY_MSEC(15)
#define LED_DELAY_LONG  PICORTOS_DELAY_MSEC(30)

static void clock_init(void)
{
    /* S160 -> 16Mhz */
    MC_CGM->SC_DC0 = (uint32_t)MC_CGM_SC_DC0_DIV(0);
    MC_CGM->SC_DC0 |= (uint32_t)MC_CGM_SC_DC0_DE(1);
    /* S40 -> 4Mhz */
    MC_CGM->SC_DC2 = (uint32_t)MC_CGM_SC_DC2_DIV(0);
    MC_CGM->SC_DC2 |= (uint32_t)MC_CGM_SC_DC2_DE(1);

    /* DRUN mode (no LP support yet) */
    MC_ME->RUN_PC[0] = (uint32_t)MC_ME_RUN_PC_DRUN(1);

    /* Switch to FIRC (16Mhz) */
    MC_ME->DRUN_MC = (uint32_t)(MC_ME_DRUN_MC_MVRON(1) |        /* Main Voltage regulator on */
                                MC_ME_DRUN_MC_FIRCON(1) |       /* 16Mhz FIRC on */
                                MC_ME_DRUN_MC_SYSCLK(0));       /* SYSCLK is FIRC */

    /* DRUN mode + key */
    MC_ME->MCTL = (uint32_t)(MC_ME_MCTL_TARGET_MODE(3) | MC_ME_MCTL_KEY(0x5af0));
    MC_ME->MCTL = (uint32_t)(MC_ME_MCTL_TARGET_MODE(3) | MC_ME_MCTL_KEY(0xa50f));
}

static void set_gpio_output(unsigned long pin)
{
    SIUL2->MSCR[pin] &= ~SIUL2_MSCR_SSS_MASK;
    SIUL2->MSCR[pin] |= SIUL2_MSCR_OBE((uint32_t)1);
}

static void hw_init(void)
{
    clock_init();

    /* LEDs */
    set_gpio_output(LED0);
    set_gpio_output(LED1);
    set_gpio_output(LED2);
    set_gpio_output(LED3);
    set_gpio_output(LED4);
    set_gpio_output(LED5);
    set_gpio_output(LED6);
    set_gpio_output(LED7);

    /* TICK */
    set_gpio_output(TICK);
}

static void set_gpio(unsigned long pin, bool status)
{
    unsigned long port = (pin >> 4);
    unsigned long mask = 1ul << (15 - (0xf & pin));

    if (status) SIUL2->PGPDO[port] |= mask;
    else SIUL2->PGPDO[port] &= ~mask;
}

static void set_led(unsigned long led, bool status, picoRTOS_tick_t delay)
{
    set_gpio(led, status);
    picoRTOS_sleep(delay);
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
        set_gpio(TICK, x);

        x = !x;
        picoRTOS_schedule();
    }
}

static void leds_main(void *priv)
{
    arch_assert(priv == NULL);

    picoRTOS_tick_t ref = picoRTOS_get_tick();

    for (;;) {
        picoRTOS_sleep_until(&ref, PICORTOS_DELAY_SEC(1));

        /* turn on */
        set_led(LED0, false, LED_DELAY_SHORT);
        set_led(LED1, false, LED_DELAY_SHORT);
        set_led(LED2, false, LED_DELAY_SHORT);
        set_led(LED3, false, LED_DELAY_SHORT);
        set_led(LED4, false, LED_DELAY_SHORT);
        set_led(LED5, false, LED_DELAY_SHORT);
        set_led(LED6, false, LED_DELAY_SHORT);
        set_led(LED7, false, LED_DELAY_SHORT);

        /* turn off */
        set_led(LED7, true, LED_DELAY_LONG);
        set_led(LED6, true, LED_DELAY_LONG);
        set_led(LED5, true, LED_DELAY_LONG);
        set_led(LED4, true, LED_DELAY_LONG);
        set_led(LED3, true, LED_DELAY_LONG);
        set_led(LED2, true, LED_DELAY_LONG);
        set_led(LED1, true, LED_DELAY_LONG);
        set_led(LED0, true, LED_DELAY_LONG);

        /* stack test */
        deepcall_schedule(10);
    }
}

int main(void)
{
    hw_init();
    picoRTOS_init();

    struct picoRTOS_task task;
    static picoRTOS_stack_t stack0[CONFIG_DEFAULT_STACK_COUNT];
    static picoRTOS_stack_t stack1[CONFIG_DEFAULT_STACK_COUNT];

    picoRTOS_task_init(&task, tick_main, NULL, stack0, CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, TASK_TICK_PRIO);
    picoRTOS_task_init(&task, leds_main, NULL, stack1, CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, TASK_LED_PRIO);

    picoRTOS_start();

    /* not supposed to end there */
    arch_assert(false);
    return 1;
}
