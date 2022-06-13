#include "picoRTOS.h"
#include "gpio-avr.h"

struct AVR_GPIO {
    unsigned char PIN;
    unsigned char DDR;
    unsigned char PORT;
};

int gpio_avr_init(struct gpio *ctx, struct AVR_GPIO *base,
                  gpio_avr_pin_t pin, gpio_avr_dir_t dir)
{
    arch_assert(pin < (gpio_avr_pin_t)8);
    arch_assert(dir <= GPIO_AVR_DIR_OUTPUT);

    ctx->base = base;
    ctx->pin = pin;

    if (dir == GPIO_AVR_DIR_OUTPUT)
        base->DDR |= (unsigned char)(1 << pin);

    return 0;
}

/* HAL */

void gpio_write(struct gpio *ctx, bool value)
{
    if (value) ctx->base->PORT |= (1 << ctx->pin);
    else ctx->base->PORT &= ~(1 << ctx->pin);
}

bool gpio_read(struct gpio *ctx)
{
    unsigned char mask = (unsigned char)(1 << ctx->pin);

    return (ctx->base->PIN & mask) != (unsigned char)0;
}
