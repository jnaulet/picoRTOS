#ifndef UART_AVR_H
#define UART_AVR_H

#include "uart.h"
#include "clock.h"

struct AVR_USART;

struct uart {
    /*@temp@*/ struct AVR_USART *base;
    clock_id_t clkid;
};

int uart_avr_init(/*@out@*/ struct uart *ctx,
                            struct AVR_USART *base,
                            clock_id_t clkid);

#endif
