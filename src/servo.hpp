#ifndef HPP_SERVO_
#define HPP_SERVO_

#define MID_DUTY 750
#define DUTY_RANGE 200
#define CLOCK_DIV 250
#define WRAP 10000
#define MID_ANGLE 0
#define ANGLE_RANGE 90

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

struct servo {
    const uint pin;
    uint slice;
    uint channel;
    

    servo(uint pin): pin(pin) {
        slice = pwm_gpio_to_slice_num(pin);
        channel = pwm_gpio_to_channel(pin);
    };

    void PWMInit() {
        gpio_set_function(pin, GPIO_FUNC_PWM);
        pwm_config config = pwm_get_default_config();
        pwm_config_set_clkdiv(&config, CLOCK_DIV);
        pwm_config_set_wrap(&config, WRAP);
        pwm_set_gpio_level(pin, MID_DUTY);
        pwm_init(slice, &config, true);
    }

    void init() {
        PWMInit();
    }

    void moveTo(int a) {
        if (a<MID_ANGLE-ANGLE_RANGE) {
            a=MID_ANGLE-ANGLE_RANGE;
        }
        if (a>MID_ANGLE+ANGLE_RANGE) {
            a=MID_ANGLE+ANGLE_RANGE;
        }
        uint16_t level = (a-(MID_ANGLE-ANGLE_RANGE))*DUTY_RANGE/ANGLE_RANGE+(MID_DUTY-DUTY_RANGE);
        pwm_set_chan_level(slice, channel, level);
    }
};
#endif