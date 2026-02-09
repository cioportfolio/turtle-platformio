#ifndef COMMON_H
#define COMMON_H
#include <SDFS.h>

// Default access point details
#define DEF_SSID "turtle"
#define DEF_PWD "turtlepassword"

// Motor pins
#define ENCODER_PIN_LA 10
#define ENCODER_PIN_RA 12
#define POWER_PIN_LA 4
#define POWER_PIN_RA 6
#define SERVO_PIN 8

#define PEN_UP -45
#define PEN_DOWN 45

// SD card holder pins
#define SPI_TX 19
#define SPI_RX 16
#define SPI_SCK 18
#define SD_CS_PIN 17

#define WIFI_FILE "wifi.tsv"

#define SSID_LENGTH 30

// UDP Beacon settings
#define BEACON_INTERVAL_MS 5000
#define BEACON_BUFFER 100
#define BEACON_PORT 4444
#define BEACON_TARGET "255.255.255.255"

#endif