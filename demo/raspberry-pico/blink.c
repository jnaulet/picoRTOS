/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "picoRTOS-SMP.h"

#include "picoRTOS_mutex.h"
#include "picoRTOS_cond.h"
#include "pico/stdlib.h"

#ifndef PICO_DEFAULT_LED_PIN
# error blink example requires a board with a regular LED
#endif

#define LED_PIN  PICO_DEFAULT_LED_PIN

struct picoRTOS_mutex mutex = PICORTOS_MUTEX_INITIALIZER;
struct picoRTOS_cond cond = PICORTOS_COND_INITIALIZER;

static void hw_init(void)
{
    /* LED */
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

static void producer_main(void *priv)
{
    arch_assert(priv == NULL);

    picoRTOS_tick_t ref = picoRTOS_get_tick();

    for (;;) {
        picoRTOS_sleep_until(&ref, PICORTOS_DELAY_MSEC(500));
        picoRTOS_mutex_lock(&mutex);
        picoRTOS_cond_signal(&cond);
        picoRTOS_mutex_unlock(&mutex);
    }
}

static void consumer_main(void *priv)
{
    arch_assert(priv == NULL);

    static int led = 0;
    for (;;) {
        picoRTOS_mutex_lock(&mutex);
        picoRTOS_cond_wait(&cond, &mutex);
        picoRTOS_mutex_unlock(&mutex);
        led ^= 1;
        gpio_put(LED_PIN, led);
    }
}

int main(void)
{
    hw_init();
    picoRTOS_init();

    struct picoRTOS_task task;
    static picoRTOS_stack_t stack0[CONFIG_DEFAULT_STACK_COUNT];
    static picoRTOS_stack_t stack1[CONFIG_DEFAULT_STACK_COUNT];

    /* shared task */
    picoRTOS_task_init(&task, producer_main, NULL, stack0, CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, TASK_TICK_PRIO);

    /* per core tasks */
    picoRTOS_task_init(&task, consumer_main, NULL, stack1, CONFIG_DEFAULT_STACK_COUNT);
    picoRTOS_add_task(&task, TASK_LED_PRIO);

    picoRTOS_start();

    /* not supposed to end there */
    arch_assert(false);
    return 1;
}
