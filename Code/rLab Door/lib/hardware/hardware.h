#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

/* GPIO config */

/* UARTs */
/* Serial - Standard pinouts for programming/logging */
#define GPIO_TXDO_PROG       1
#define GPIO_RXDI_PROG       3
/* Serial1 - Raspberry Pi Serial Interface */
#define GPIO_TXDO_PI         18
#define GPIO_RXDI_PI         19
/* Serial2 - NFC Serial Interface */
#define GPIO_TXDO_NFC        26
#define GPIO_RXDI_NFC        27

/* Raspberry Pi */
#define GPIO_PI_HEARTBEAT    16
#define GPIO_PI_POWERED      17
#define GPIO_PI_OPEN_CMD_1   23
#define GPIO_PI_OPEN_CMD_2   25

/* Monitoring */
#define GPIO_TAMPER          15
#define GPIO_ADC_VIN         35
#define GPIO_ADC_VBATT       34
#define GPIO_ADC_V5          32
#define GPIO_ADC_V3          33

/* Display */
#define GPIO_NPX_1           5
#define GPIO_NPX_2           4
#define GPIO_SDA_DISP        21
#define GPIO_SCL_DISP        22

/* Control */
#define GPIO_OPEN_1          13
#define GPIO_OPEN_2          2

/* Keypad */
#define GPIO_WGD0            14
#define GPIO_WGD1            12

/* I2C Character Display */
#define DISP_ADDR           0x27
#define DISP_ROWS           4
#define DISP_COLS           20
#define SSD_ADDR            0x3C
#define SSD_ROWS            64
#define SSD_COLS            128

/* Neopixels */
#define NPX_NUM_LEDS_1      6
#define NPX_NUM_LEDS_2      0

/* Hardware Enabled Bits */
#define FEATURE_PI          0x01    // Monitor and accept commands from the Pi
#define FEATURE_DISP        0x02    // Send messages to character display
#define FEATURE_NPX_1       0x04    // Drive neopixel string 1
#define FEATURE_NPX_2       0x08    // Drive neopixel string 2
#define FEATURE_OPEN_2      0x10    // OPEN_1 always enabled
#define FEATURE_KEYPAD      0x20    // Accept input from the keypad
#define FEATURE_NFC         0x40    // Accept input from the NFC reader
#define FEATURE_WIFI        0x80    // Allow WiFi connectivity
#define FEATURE_PUSHOVER    0x10    // Allow notify to pushover (Needs wifi)

/* Int for features enabled/configured and OK */
extern uint32_t enabled;
extern uint32_t configured;
extern uint32_t status;

/* Max variable string lengths */
/* Wifi */
#define WIFI_SSID_MAX_LEN   16
#define WIFI_PASSWD_MAX_LEN 32

void setup_gpio();
void load_prefs();
void show_features(HardwareSerial * console);
int configure_wifi(char * ssid, char * passwd);
void start_wifi();

#endif