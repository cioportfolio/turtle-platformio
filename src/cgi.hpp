#define SM_L 0
#define SM_R 1
#define ENCODER_PIN_LA 10
#define ENCODER_PIN_RA 12
#define POWER_PIN_LA 4
#define POWER_PIN_RA 6
#define SERVO_PIN 8

#define PEN_UP -45
#define PEN_DOWN 45

#include <string>

#include "motor.hpp"
#include "servo.hpp"
#include "web.hpp"

servo serv = servo(SERVO_PIN);
motor left_motor = motor(POWER_PIN_LA, ENCODER_PIN_LA, SM_L);
motor right_motor = motor(POWER_PIN_RA, ENCODER_PIN_RA, SM_R);
motor_pair motors = motor_pair(left_motor, right_motor);

// CGI handler which is run when a request for /cmd.cgi is detected
const char *cgi_cmd_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    if (strcmp(pcParam[0], "move") == 0) {
        double rev = atof(pcValue[0]);
        motors.forwardBy(rev);
    }
    if (strcmp(pcParam[0], "turn") == 0) {
        double rev = atof(pcValue[0]);
        motors.panBy(rev);
    }
    if (strcmp(pcParam[0], "pen") == 0) {
        int deg = atoi(pcValue[0]);
        serv.moveTo(deg);
    }
    beacon=false;
    // Send the index page back to the user
    return "/index.shtml";
}

const char *cgi_wifi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    if (iNumParams==2) {
        for(int i=0; i<2;i++) {
            if (strcmp(pcParam[i],"ssid")==0) {
                strncpy(ssid, pcValue[i],32);
                ssid[32]='\0';
            } else {
                if (strcmp(pcParam[i],"pwd")==0) {
                    strncpy(pwd, pcValue[i],32);
                    pwd[32]='\0';
                }
            }
        }
        saveWifi();
    }
    // Send the index page back to the user
    return "/reboot.shtml";
}

// tCGI Struct
// Fill this with all of the CGI requests and their respective handlers
static const tCGI cgi_handlers[] = {
    {// Html request for "/cmd.cgi" triggers cgi_cmd_handler
     "/cmd.cgi", cgi_cmd_handler},
     {"/wifi.cgi", cgi_wifi_handler}
};

void cgi_init(void) {
    serv.init();
    motors.init();
    http_set_cgi_handlers(cgi_handlers, 2);
}