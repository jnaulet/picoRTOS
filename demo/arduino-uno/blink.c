#include "picoRTOS.h"
#include "gpio-avr.h"
#include "uart-avr.h"
#include "clock-atmega328p.h"

#define PTD 0x29
#define PTB 0x23
#define USART0 0xc0
#define USART1 0xc8

/* Beware: avr-gcc doesn't optimize out the delay computation */
#define BLINK_PERIOD PICORTOS_DELAY_SEC(1)
#define BLINK_DELAY  PICORTOS_DELAY_MSEC(30l)
#define CONSOLE_DELAY PICORTOS_DELAY_MSEC(40l)

/* Tasks contexts */

static struct task_tick {
    struct gpio gpio;
} tick;

static struct task_blink {
    struct gpio gpio;
} blink;

static struct task_console {
    struct uart uart;
} console;

static void hw_init(void)
{
    /* set tasks GPIOs */
    gpio_avr_init(&tick.gpio, (struct AVR_GPIO*)PTD, (gpio_avr_pin_t)2, GPIO_AVR_DIR_OUTPUT);
    gpio_avr_init(&blink.gpio, (struct AVR_GPIO*)PTB, (gpio_avr_pin_t)5, GPIO_AVR_DIR_OUTPUT);
    /* setup console */
    uart_avr_init(&console.uart, (struct AVR_USART*)USART0, CLOCK_ATMEGA328P_CLKIO);

    struct uart_settings settings = { 9600ul, (size_t)8, false, false, false };
    (void)uart_setup(&console.uart, &settings);
}

static void deepcall_schedule(unsigned int n)
{
    if (n != 0)
        deepcall_schedule(n - 1);
    else
        picoRTOS_schedule();
}

static void tick_main(void *priv)
{
    struct task_tick *ctx = (struct task_tick*)priv;

    arch_assert(ctx != NULL);

    bool x = false;

    for (;;) {
        /* toggle pin */
        gpio_write(&ctx->gpio, x);

        /* stack test */
        deepcall_schedule(20u);
        x = !x;
    }
}

static void blink_main(void *priv)
{
    picoRTOS_tick_t ref = picoRTOS_get_tick();
    struct task_blink *ctx = (struct task_blink*)priv;

    arch_assert(ctx != NULL);

    for (;;) {
        picoRTOS_sleep_until(&ref, BLINK_PERIOD);

        /* blink */
        gpio_write(&ctx->gpio, true);
        picoRTOS_sleep(BLINK_DELAY);
        gpio_write(&ctx->gpio, false);
        picoRTOS_sleep(BLINK_DELAY);

        gpio_write(&ctx->gpio, true);
        picoRTOS_sleep(BLINK_DELAY);
        picoRTOS_sleep(BLINK_DELAY);
        gpio_write(&ctx->gpio, false);

        /* stack test */
        deepcall_schedule(10u);
    }
}

static void console_main(void *priv)
{
    struct task_console *ctx = (struct task_console*)priv;

    arch_assert(ctx != NULL);

    for (;;) {

        char c = (char)0;

        /* just echo */
        if (uart_read_wait(&ctx->uart, &c, sizeof(c),
                           PICORTOS_DELAY_SEC(1)) == (int)sizeof(c))
            (void)uart_write_wait(&ctx->uart, &c, sizeof(c),
                                  PICORTOS_DELAY_SEC(1));
    }
}

int main( void )
{
    hw_init();
    picoRTOS_init();

    struct picoRTOS_task task;
    static picoRTOS_stack_t stack0[CONFIG_DEFAULT_STACK_COUNT];
    static picoRTOS_stack_t stack1[CONFIG_DEFAULT_STACK_COUNT];
    static picoRTOS_stack_t stack2[CONFIG_DEFAULT_STACK_COUNT];

    picoRTOS_task_init(&task, tick_main, NULL, stack0, (size_t)CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, (picoRTOS_priority_t)TASK_TICK_PRIO);
    picoRTOS_task_init(&task, blink_main, NULL, stack1, (size_t)CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, (picoRTOS_priority_t)TASK_BLINK_PRIO);
    picoRTOS_task_init(&task, console_main, &console, stack2, (picoRTOS_size_t)CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, (picoRTOS_priority_t)TASK_CONSOLE_PRIO);

    /* Start the scheduler. */
    picoRTOS_start();

    /* we're not supposed to end here */
    arch_assert(false);
    return -1;
}
