#ifndef CONTROL_H
#define CONTROL_H

#include "common.hpp"
#include "motor.hpp"
#include "servo.hpp"

servo serv = servo(SERVO_PIN);
motor left_motor = motor('L', POWER_PIN_LA, ENCODER_PIN_LA, SM_L);
motor right_motor = motor('R', POWER_PIN_RA, ENCODER_PIN_RA, SM_R);
motor_pair motors = motor_pair(left_motor, right_motor);

void initControl(void) {
//    Serial.println("Pause...");
//    delay(5000);
    serv.init();
    Serial.println("Servo inited...");
//    delay(5000);
    left_motor.init();
    Serial.println("Left motor inited...");
//    delay(5000);
    right_motor.init();
    Serial.println("Right motor inited");
}

#endif