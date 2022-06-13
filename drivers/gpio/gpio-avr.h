#ifndef GPIO_AVR_H
#define GPIO_AVR_H

#include "gpio.h"

typedef unsigned char gpio_avr_pin_t;
typedef unsigned char gpio_avr_dir_t;

struct AVR_GPIO;

struct gpio {
    /*@temp@*/ struct AVR_GPIO *base;
    gpio_avr_pin_t pin;
};

#define GPIO_AVR_DIR_INPUT (gpio_avr_dir_t)0
#define GPIO_AVR_DIR_OUTPUT (gpio_avr_dir_t)1

int gpio_avr_init(/*@out@*/ struct gpio *ctx,
                            struct AVR_GPIO *base,
                            gpio_avr_pin_t pin,
                            gpio_avr_dir_t dir);

#endif
