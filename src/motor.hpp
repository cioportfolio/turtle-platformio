#ifndef HPP_MOTOR_
#define HPP_MOTOR_

#define MOTOR_CLOCK_DIV 250
#define MOTOR_WRAP 10000
#define MAX_LEVEL 10000
#define GEAR_RATIO 150
#define STEPS_PER_REV 28
#define DEFAULT_PIO pio1
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
    char label;
    int stepThreshold=DEFAULT_STEP_THRESHOLD;
    int crawlThreshold=DEFAULT_CRAWL_THRESHOLD;
    uint8_t fullPower=DEFAULT_FULL_POWER;
    uint8_t crawlPower=DEFAULT_CRAWL_POWER;

    PIO pio=DEFAULT_PIO;
    uint sm;

    motor(char label, uint firstPowerPin, uint firstEncoderPin, uint sm, uint max_level=MAX_LEVEL): label(label), driveA(firstPowerPin), encodeA(firstEncoderPin), sm(sm), max_level(max_level) {
        slice1 = pwm_gpio_to_slice_num(driveA);
        channel1 = pwm_gpio_to_channel(driveA);        
        slice2 = pwm_gpio_to_slice_num(driveA+1);
        channel2 = pwm_gpio_to_channel(driveA+1);  
    };

    void PWMsInit(uint firstpin) {
        gpio_set_function(firstpin, GPIO_FUNC_PWM);
        gpio_set_function(firstpin+1, GPIO_FUNC_PWM);
        pwm_config config = pwm_get_default_config();
        pwm_config_set_clkdiv(&config, MOTOR_CLOCK_DIV);
        pwm_config_set_wrap(&config, MOTOR_WRAP);
        pwm_set_gpio_level(firstpin, 0);
        pwm_set_gpio_level(firstpin+1, 0);
        pwm_init(slice1, &config, true);
        pwm_init(slice2, &config, true);
        Serial.print("Motor ");
        Serial.print(label);
        Serial.println(" PMW init");
    }

    void EncInit(uint pin) {
        pio_add_program(pio, &quadrature_encoder_program);
        uint offset=0;
        quadrature_encoder_program_init(pio, sm, encodeA, 0);
        Serial.print("Motor ");
        Serial.print(label);
        Serial.print("Enc PIO:");
        Serial.print(pio==pio0?0:1);
        Serial.print(" state machine:");
        Serial.println(sm);
    }

    void init() {
        Serial.print("Motor ");
        Serial.print(label);
        Serial.print(" PMW 1 slice:");
        Serial.print(slice1);
        Serial.print(" Channel:");
        Serial.print(channel1);
        Serial.print(" PMW 2 slice:");
        Serial.print(slice2);
        Serial.print(" Channel:");
        Serial.println(channel2);
        PWMsInit(driveA);
        EncInit(encodeA);
    }

    void start(bool dir, uint8_t pwr) {
        uint16_t level=pwr*max_level/fullPower;
/*        Serial.print("Motor on pin ");
        Serial.print(driveA);
        Serial.println(" start");*/
        pwm_set_chan_level(slice1, channel1, dir?level:0);
        pwm_set_chan_level(slice2, channel2, dir?0:level);
    }

    void start(bool dir) {
        start(dir, fullPower);
    }

    void stop() {
        Serial.print("Motor ");
        Serial.print(label);
        Serial.println(" stop");
        pwm_set_gpio_level(driveA, 0);
        pwm_set_gpio_level(driveA+1, 0);
    }

    int position() {
        return quadrature_encoder_get_count(pio, sm);
    }

    void turnTo(int step) {
        Serial.print("Motor ");
        Serial.print(label);
        Serial.print(" turn to ");
        Serial.println(step);
        int curr=position();
        int gap=abs(step-curr);
        if (gap<=stepThreshold) return;
        while (gap>stepThreshold) {
            start(step>curr, gap<crawlThreshold?crawlPower:fullPower);
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
    int stepThreshold=DEFAULT_STEP_THRESHOLD;
    int crawlThreshold=DEFAULT_CRAWL_THRESHOLD;
    uint8_t fullPower=DEFAULT_FULL_POWER;
    uint8_t crawlPower=DEFAULT_CRAWL_POWER;


    motor_pair(motor left, motor right): left(left), right(right) {};

    void init() {
        left.init();
        right.init();
    }

    void turnBy(int leftSteps, int rightSteps) {
        Serial.print("Motor pair turn by (");
        Serial.print(leftSteps);
        Serial.print(",");
        Serial.print(rightSteps);
        Serial.println(")");

        int lg = abs(leftSteps);
        int rg = abs(rightSteps);
        Serial.print("Motor pair position\n(");
        int lc = left.position();
        Serial.print(lc);
        Serial.println(",");
        int lt = lc+leftSteps;
        int rc = right.position();
        Serial.print(rc);
        Serial.println(")");
        int rt = rc+rightSteps;
        int loopCount=0;
        while (lg>stepThreshold && rg>stepThreshold) {
            uint8_t lp=lg<crawlThreshold?crawlPower:fullPower;
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
            Serial.print("\rMotor pair pos (");
            Serial.print(lc);
            Serial.print(",");
            Serial.print(rc);
            Serial.print(") gaps (");
            Serial.print(lg);
            Serial.print(",");
            Serial.print(rg);
            Serial.print(") count ");
            Serial.print(++loopCount);   
        }
        if (lg>stepThreshold || rg>stepThreshold) {
            if (lg>stepThreshold) {
                right.stop();
                left.turnTo(lt);
            } else {
                left.stop();
                right.turnTo(rt);
            }
            return;
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