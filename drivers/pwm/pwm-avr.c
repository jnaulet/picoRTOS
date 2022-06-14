#include "pwm-avr.h"
#include <errno.h>

struct AVR_PWM {
    unsigned char TCRRxA;
    unsigned char TCRRxB;
    unsigned char TCNTx;
    unsigned char OCRxA;
    unsigned char OCRxB;
};

#define TCRRxA_COMxAn 6
#define TCRRxA_COMxBn 4
#define TCRRxB_WGMx2  3

int pwm_avr_init(struct pwm *ctx,
                 struct AVR_PWM *base,
                 pwm_avr_mode_t mode,
                 pwm_avr_prescaler_t prescaler)
{
    arch_assert(mode < PWM_AVR_MODE_COUNT);
    arch_assert(prescaler < PWM_AVR_PRESCALER_COUNT);

    ctx->base = base;
    ctx->mode = mode;
    ctx->prescaler = prescaler;

    switch (mode) {
    case PWM_AVR_NORMAL_MODE:           /*@fallthrough@*/
    case PWM_AVR_PHASE_CORRECT_FF_MODE: /*@fallthrough@*/
    case PWM_AVR_CTC_OCRA_MODE:         /*@fallthrough@*/
    case PWM_AVR_FAST_FF_MODE:
        base->TCRRxA = (unsigned char)mode;
        break;

    case PWM_AVR_PHASE_CORRECT_OCRA_MODE:
        base->TCRRxA = (unsigned char)0x1;
        base->TCRRxB = (unsigned char)0x8;
        break;

    case PWM_AVR_FAST_OCRA_MODE:
        base->TCRRxA = (unsigned char)0x3;
        base->TCRRxB = (unsigned char)0x8;
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

void pwm_start(struct pwm *ctx)
{
    ctx->base->TCRRxB = (unsigned char)ctx->prescaler;
}

void pwm_stop(struct pwm *ctx)
{
    ctx->base->TCRRxB = (unsigned char)0;
}

void pwm_set_cmpa(struct pwm *ctx, uint16_t value, pwm_act_t act)
{
    arch_assert(act < PWM_ACT_COUNT);

    ctx->base->OCRxA = (unsigned char)(value >> 8);
    ctx->base->TCRRxA &= ~(0x3 << TCRRxA_COMxAn);

    switch (act) {
    case PWM_ACT_TOGGLE:
        ctx->base->TCRRxA |= (0x1 << TCRRxA_COMxAn);
        break;

    case PWM_ACT_CLEAR:
        ctx->base->TCRRxA |= (0x2 << TCRRxA_COMxAn);
        break;

    default:
        ctx->base->TCRRxA |= (0x3 << TCRRxA_COMxAn);
        break;
    }
}

void pwm_set_cmpb(struct pwm *ctx, uint16_t value, pwm_act_t act)
{
    arch_assert(act < PWM_ACT_COUNT);

    ctx->base->OCRxB = (unsigned char)(value >> 8);
    ctx->base->TCRRxA &= ~(0x3 << TCRRxA_COMxBn);

    switch (act) {
    case PWM_ACT_TOGGLE:
        arch_assert(ctx->mode == PWM_AVR_NORMAL_MODE);
        ctx->base->TCRRxA |= (0x1 << TCRRxA_COMxBn);
        break;

    case PWM_ACT_CLEAR:
        ctx->base->TCRRxA |= (0x2 << TCRRxA_COMxBn);
        break;

    default:
        ctx->base->TCRRxA |= (0x3 << TCRRxA_COMxBn);
        break;
    }
}
