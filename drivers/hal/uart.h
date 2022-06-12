#ifndef HAL_UART_H
#define HAL_UART_H

#include "picoRTOS.h"
#include <stdbool.h>

struct uart;

struct uart_settings {
    unsigned long baudrate;
    size_t cs;
    bool cstopb;
    bool parenb;
    bool parodd;
};

/* ARCH specific functions */
int uart_setup(struct uart *ctx, const struct uart_settings *settings);
int uart_write(struct uart *ctx, const char *buf, size_t n);
int uart_read(struct uart *ctx, char *buf, size_t n);

#endif
