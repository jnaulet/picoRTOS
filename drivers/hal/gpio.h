#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>

struct gpio;

void gpio_write(struct gpio *ctx, bool value);
bool gpio_read(struct gpio *ctx);

#endif
