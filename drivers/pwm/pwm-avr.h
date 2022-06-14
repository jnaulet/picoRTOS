#ifndef PWM_AVR_H
#define PWM_AVR_H

#include "picoRTOS.h"
#include "pwm.h"

struct AVR_PWM;

typedef enum {
    PWM_PRESCALER_NO_CLOCK  = 0,
    PWM_AVR_PRESCALER_1     = 1,
    PWM_AVR_PRESCALER_8     = 2,
    PWM_AVR_PRESCALER_64    = 3,
    PWM_AVR_PRESCALER_256   = 4,
    PWM_AVR_PRESCALER_1024  = 5,
    PWM_AVR_PRESCALER_COUNT
} pwm_avr_prescaler_t;

typedef enum {
    PWM_AVR_NORMAL_MODE             = 0,
    PWM_AVR_PHASE_CORRECT_FF_MODE   = 1,
    PWM_AVR_CTC_OCRA_MODE           = 2,
    PWM_AVR_FAST_FF_MODE            = 3,
    PWM_AVR_PHASE_CORRECT_OCRA_MODE = 5,
    PWM_AVR_FAST_OCRA_MODE          = 7,
    PWM_AVR_MODE_COUNT
} pwm_avr_mode_t;

struct pwm {
    /*@temp@*/ struct AVR_PWM *base;
    pwm_avr_prescaler_t prescaler;
    pwm_avr_mode_t mode;
};

int pwm_avr_init(/*@out@*/ struct pwm *ctx,
                           struct AVR_PWM *base,
                           pwm_avr_mode_t mode,
                           pwm_avr_prescaler_t prescaler);

#endif
