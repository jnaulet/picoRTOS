#ifndef HAL_PWM_H
#define HAL_PWM_H

#include "picoRTOS.h"
#include <stdint.h>

struct pwm;

typedef enum {
    PWM_ACT_TOGGLE,
    PWM_ACT_CLEAR,
    PWM_ACT_SET,
    PWM_ACT_COUNT
} pwm_act_t;

#define PWM_CMP_VALUE_0  0
#define PWM_CMP_VALUE_100 65535u

void pwm_start(struct pwm *ctx);
void pwm_stop(struct pwm *ctx);
void pwm_set_cmpa(struct pwm *ctx, uint16_t value, pwm_act_t act);
void pwm_set_cmpb(struct pwm *ctx, uint16_t value, pwm_act_t act);

#endif
