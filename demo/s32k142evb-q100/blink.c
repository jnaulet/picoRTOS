#include "picoRTOS.h"
#include <stdbool.h>

#include "device_registers.h"

#define BLINK_PERIOD PICORTOS_DELAY_SEC(1)
#define BLINK_DELAY  PICORTOS_DELAY_MSEC(60)

#define LED_RED   15
#define LED_GREEN 16
#define LED_BLUE  0

#define PTD_LED_RED   (1u << LED_RED)
#define PTD_LED_GREEN (1u << LED_GREEN)
#define PTD_LED_BLUE  (1u << LED_BLUE)

#define TICK 1
#define PTD_TICK (1u << TICK)

static void isr(void *priv)
{
    arch_assert(priv == NULL);

    PTD->PTOR = (uint32_t)PTD_LED_RED;
}

static void hw_init(void)
{
    /* clocks */
    PCC->PCCn[PCC_PORTD_INDEX] = (uint32_t)PCC_PCCn_CGC_MASK;

    /* PORTD configuration */
    PORTD->PCR[TICK] = (uint32_t)PORT_PCR_MUX(1);
    PORTD->PCR[TICK] |= PORT_PCR_DSE_MASK | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

    PORTD->PCR[LED_RED] = (uint32_t)PORT_PCR_MUX(1);
    PORTD->PCR[LED_GREEN] = (uint32_t)PORT_PCR_MUX(1);
    PORTD->PCR[LED_BLUE] = (uint32_t)PORT_PCR_MUX(1);

    /* outputs */
    PTD->PDDR |= PTD_TICK | PTD_LED_RED | PTD_LED_GREEN | PTD_LED_BLUE;
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

    for (;;) {
        /* toggle pin */
        PTD->PTOR = (uint32_t)PTD_TICK;

        /* stack test */
        deepcall_schedule(20);
    }
}

static void blink_main(void *priv)
{
    picoRTOS_tick_t ref = picoRTOS_get_tick();

    arch_assert(priv == NULL);

    for (;;) {
        picoRTOS_sleep_until(&ref, BLINK_PERIOD);

        /* blink */
        S32_NVIC->ISPR[1] |= (1 << 30);
        picoRTOS_sleep(BLINK_DELAY);
        S32_NVIC->ISPR[1] |= (1 << 30);
        PTD->PSOR = (uint32_t)PTD_LED_GREEN;
        picoRTOS_sleep(BLINK_DELAY);
        PTD->PCOR = (uint32_t)PTD_LED_GREEN;
        PTD->PSOR = (uint32_t)PTD_LED_BLUE;
        picoRTOS_sleep(BLINK_DELAY);
        PTD->PCOR = (uint32_t)PTD_LED_BLUE;

        /* stack test */
        deepcall_schedule(10);
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

    /* interrupt test */
    picoRTOS_register_interrupt(PORTD_IRQn, isr, NULL);
    picoRTOS_enable_interrupt(PORTD_IRQn);

    /* Start the scheduler. */
    picoRTOS_start();

    /* we're not supposed to end here */
    arch_assert(false);
    return -1;
}
