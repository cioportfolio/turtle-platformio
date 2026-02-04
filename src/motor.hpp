#ifndef HPP_MOTOR_
#define HPP_MOTOR_

#define CLOCK_DIV 250
#define WRAP 10000
#define MAX_LEVEL 10000
#define GEAR_RATIO 150
#define STEPS_PER_REV 28
#define STEP_THRESHOLD 10
#define CRAWL_THRESHOLD (GEAR_RATIO*STEPS_PER_REV/4)
#define FULL_POWER 255
#define CRAWL_POWER 85 // approx 30%
#define DEFAULT_PIO pio0
#define DEFAULT_SM 0

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

#include "quadrature_encoder.pio.h"

int revToStep(double rev) {
    return rev*GEAR_RATIO*STEPS_PER_REV;
}

int abs(int in) {
    if (in<0) return -in;
    return in;
}

struct motor {
    const uint driveA;
    uint slice1,slice2;
    uint channel1,channel2;
    const uint encodeA;
    const uint max_level;

    const PIO pio = DEFAULT_PIO;
    const uint sm;

    motor(uint firstPowerPin, uint firstEncoderPin, uint stateMachine = DEFAULT_SM, uint max_level=MAX_LEVEL): driveA(firstPowerPin), encodeA(firstEncoderPin), max_level(max_level), sm(stateMachine) {
        slice1 = pwm_gpio_to_slice_num(driveA);
        channel1 = pwm_gpio_to_channel(driveA);        
        slice2 = pwm_gpio_to_slice_num(driveA+1);
        channel2 = pwm_gpio_to_channel(driveA+1);        
    };

    void PWMsInit(uint firstpin) {
        gpio_set_function(firstpin, GPIO_FUNC_PWM);
        gpio_set_function(firstpin+1, GPIO_FUNC_PWM);
        pwm_config config = pwm_get_default_config();
        pwm_config_set_clkdiv(&config, CLOCK_DIV);
        pwm_config_set_wrap(&config, WRAP);
        pwm_set_gpio_level(firstpin, 0);
        pwm_set_gpio_level(firstpin+1, 0);
        pwm_init(slice1, &config, true);
        pwm_init(slice2, &config, true);
    }

    void EncInit(uint pin) {
        pio_add_program(pio, &quadrature_encoder_program);
        quadrature_encoder_program_init(pio, sm, encodeA, 0);
    }

    void init() {
        PWMsInit(driveA);
        EncInit(encodeA);
    }

    void start(bool dir, uint8_t pwr) {
        uint16_t level=pwr*max_level/FULL_POWER;
        pwm_set_chan_level(slice1, channel1, dir?level:0);
        pwm_set_chan_level(slice2, channel2, dir?0:level);
    }

    void start(bool dir) {
        start(dir, FULL_POWER);
    }

    void stop() {
        pwm_set_gpio_level(driveA, 0);
        pwm_set_gpio_level(driveA+1, 0);
    }

    int position() {
        return quadrature_encoder_get_count(pio, sm);
    }

    void turnTo(int step) {
        int curr=position();
        int gap=abs(step-curr);
        if (gap<=STEP_THRESHOLD) return;
        while (gap>STEP_THRESHOLD) {
            start(step>curr, gap<CRAWL_THRESHOLD?CRAWL_POWER:FULL_POWER);
            curr=position();
            gap=abs(step-curr);
        }
        stop();
    }

    void turnTo(double rev) {
        turnTo(revToStep(rev));
    }

    void turnBy(int step) {
        turnTo(position()+step);
    }

    void turnBy(double rev) {
        turnBy(revToStep(rev));
    }
};

struct motor_pair {
    motor left, right;

    motor_pair(motor left, motor right): left(left), right(right) {};

    void init() {
        left.init();
        right.init();
    }

    void turnBy(int leftSteps, int rightSteps) {
        int lg = abs(leftSteps);
        int rg = abs(rightSteps);
        if (lg<STEP_THRESHOLD && rg<STEP_THRESHOLD) {
            return;
        }
        if (lg<STEP_THRESHOLD || rg<STEP_THRESHOLD) {
            if (lg<STEP_THRESHOLD) {
                right.turnBy(rightSteps);
            } else {
                left.turnBy(leftSteps);
            }
            return;
        }
        int lc = left.position();
        int lt = lc+leftSteps;
        int rc = right.position();
        int rt = rc+rightSteps;
        while (lg>STEP_THRESHOLD || rg>STEP_THRESHOLD) {
            uint8_t lp=lg<CRAWL_THRESHOLD?CRAWL_POWER:FULL_POWER;
            uint8_t rp=lp;
            if(lg!=rg) {
                if (lg>rg) {
                    rp=rp*rg/lg;
                } else {
                    lp=lp*lg/rg;
                }
            }
            left.start(lt>lc,lp);
            right.start(rt>rc,rp);
            lc=left.position();
            rc=right.position();
            lg=abs(lt-lc);
            rg=abs(rt-rc);
        }
        left.stop();
        right.stop();
    }

    void turnBy(double leftrev, double rightrev) {
        turnBy(revToStep(leftrev), revToStep(rightrev));
    }

    void forwardBy(int steps) {
        turnBy(steps, -steps);
    }

    void forwardBy(double rev) {
        forwardBy(revToStep(rev));
    }

    void panBy(int steps) {
        turnBy(steps,steps);
    }

    void panBy(double rev) {
        panBy(revToStep(rev));
    }

};
#endif