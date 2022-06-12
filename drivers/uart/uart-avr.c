#include "picoRTOS.h"
#include "uart-avr.h"

#include <errno.h>

struct AVR_USART {
    unsigned char UCSRxA;
    unsigned char UCSRxB;
    unsigned char UCSRxC;
    unsigned char RESERVED;
    unsigned char UBRRxL;
    unsigned char UBRRxH;
    unsigned char UDRx;
};

#define UCSRxA_U2X  (1 << 1)
#define UCSRxA_UDRE (1 << 5)
#define UCSRxA_RXC  (1 << 7)
#define UCSRxB_TXEN (1 << 3)
#define UCSRxB_RXEN (1 << 4)
#define UCSRxC_UPM1 (1 << 5)
#define UCSRxC_UPM0 (1 << 4)
#define UCSRxC_USBS (1 << 3)

static int set_baudrate(struct uart *ctx, unsigned long baud)
{
    clock_freq_t freq = clock_get_freq(ctx->clkid);
    unsigned long ubrr = (unsigned long)freq / 16ul / baud - 1;

    arch_assert(freq > 0);

    ctx->base->UBRRxH = (unsigned char)(ubrr >> 8);
    ctx->base->UBRRxL = (unsigned char)ubrr;

    /* TODO: check freq match */
    return 0;
}

/* public */

int uart_avr_init(struct uart *ctx, struct AVR_USART *base, clock_id_t clkid)
{
    ctx->base = (struct AVR_USART*)base;
    ctx->clkid = clkid;

    /* turn on */
    ctx->base->UCSRxB = (unsigned char)(UCSRxB_TXEN | UCSRxB_RXEN);
    return 0;
}

/* HAL */

int uart_setup(struct uart *ctx, const struct uart_settings *settings)
{
    int res;
    unsigned char ucsrxc = (unsigned char)0;

    if ((res = set_baudrate(ctx, settings->baudrate)) < 0)
        return res;

    if (settings->cstopb) ucsrxc |= UCSRxC_USBS;
    if (settings->parenb) {
        ucsrxc = (unsigned char)UCSRxC_UPM0;
        if (settings->parodd) ucsrxc |= UCSRxC_UPM0;
    }

    arch_assert(settings->cs > (size_t)4);
    arch_assert(settings->cs < (size_t)9);

    switch (settings->cs) {
    case 5: break;
    case 6: ucsrxc |= 0x2; break;
    case 7: ucsrxc |= 0x4; break;
    case 8: ucsrxc |= 0X6; break;
    default:
        return -EINVAL;
    }

    ctx->base->UCSRxC = ucsrxc;
    return 0;
}

int uart_write(struct uart *ctx, const char *buf, size_t n)
{
    arch_assert(n > 0);

    int nwritten = 0;

    while (n != 0) {

        if ((ctx->base->UCSRxA & UCSRxA_UDRE) == (unsigned char)0)
            break;

        ctx->base->UDRx = (unsigned char)*buf++;
        nwritten++;
        n--;
    }

    if (nwritten == 0)
        return -EAGAIN;

    return nwritten;
}

int uart_read(struct uart *ctx, char *buf, size_t n)
{
    arch_assert(n > 0);

    int nread = 0;

    while (n != 0) {

        if ((ctx->base->UCSRxA & UCSRxA_RXC) == (unsigned char)0)
            break;

        *buf++ = (char)ctx->base->UDRx;
        nread++;
        n--;
    }

    if (nread == 0)
        return -EAGAIN;

    return nread;
}
