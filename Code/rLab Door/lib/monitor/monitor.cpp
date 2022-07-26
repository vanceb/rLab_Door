#include <Arduino.h>
#include <FastLED.h>
#include <LiquidCrystal_I2C.h>
#include "uptime.h"

#include <monitor.h>
#include <pi_control.h>
#include <rfid.h>
#include <conf.h>
#include <features.h>

/* Global state variables */
static bool open1_state = true;
static bool open2_state = true;
static unsigned long open1_changed = 0;
static unsigned long open2_changed = 0;
static bool input_power_lost = false;
static bool battery_low = false;
static bool tamper = false;

/* Display */
LiquidCrystal_I2C display = LiquidCrystal_I2C(DISP_ADDR, DISP_COLS, DISP_ROWS);

int setup_i2c_disp() {
    if (enabled & FEATURE_DISP) {   
        Wire.begin(GPIO_SDA_DISP, GPIO_SCL_DISP);
        /* Check to see that we have a display */
        Wire.beginTransmission(DISP_ADDR);
        if (Wire.endTransmission() == 0) {
            log_i("Character display found at i2c 0x%02X", DISP_ADDR);
            display.init();
            display.backlight();

            return true;
        } else {
            log_w("Character display NOT found at i2c 0x%02X", DISP_ADDR);
            log_i("Scanning...");
            int error;
            int found = 0;
            for (int address = 1; address < 127; address++ )
            {
                // The i2c_scanner uses the return value of
                // the Write.endTransmisstion to see if
                // a device did acknowledge to the address.
                Wire.beginTransmission(address);
                error = Wire.endTransmission();

                if (error == 0) {
                    log_i("I2C device found at address 0x%02X", address);
                    found++;
                } else if (error == 4) {
                    log_w("Unknown error at address 0x%02X", address);
                }
            }
            log_i("Found %d devices on the I2C bus", found);
        }
    }
    return false;
}

/* check_door_state()
 * 
 * Check and control the state of the door.
 * Looks at the command variables coming from the Pi and the RFID reader
 * and opens or closes the door based on these commands.
 * It also has a safety timeout which closes the door 
 * in case it is left in an open state for too long.
 */
void check_door_state() {
    /* Change the state of the door if Pi says so */
    if (open1_state != pi_open1) {
        digitalWrite(GPIO_OPEN_1, pi_open1);
        open1_state = pi_open1;
        rfid_open1 = pi_open1;  // Avoid conflicts
        open1_changed = millis();
        if (open1_state) {
            log_i("Door 1 opened by Pi");
        } else {
            log_i("Door 1 closed by Pi");
        }
    }

    if (open2_state != pi_open2) {
        digitalWrite(GPIO_OPEN_2, pi_open2);
        open2_state = pi_open2;
        rfid_open2 = pi_open2;  // Avoid conflicts
        open2_changed = millis();
        if (open2_state) {
            log_i("Door 2 opened by Pi");
        } else {
            log_i("Door 2 closed by Pi");
        }
    }

    /* Change the state of the door if RFID says so */
    if (open1_state != rfid_open1) {
        digitalWrite(GPIO_OPEN_1, rfid_open1);
        open1_state = rfid_open1;
        pi_open1 = rfid_open1;  // Avoid conflicts
        open1_changed = millis();
        if (open1_state) {
            log_i("Door 1 opened by RFID");
        } else {
            log_i("Door 1 closed by RFID");
        }
    }

    if (open2_state != rfid_open2) {
        digitalWrite(GPIO_OPEN_2, rfid_open2);
        open2_state = rfid_open2;
        pi_open2 = rfid_open2;  // Avoid conflicts
        open2_changed = millis();
        if (open2_state) {
            log_i("Door 2 opened by RFID");
        } else {
            log_i("Door 2 closed by RFID");
        }
    }

    /* Check to see whether we should close the door based on timeout */
    if (millis() - open1_changed > DOOR_OPEN_MAX_MS && digitalRead(GPIO_OPEN_1)) {
        digitalWrite(GPIO_OPEN_1, false);
        pi_open1 = false;
        log_i("Door 1 closed because of timeout");
    }

    if (millis() - open2_changed > DOOR_OPEN_MAX_MS && digitalRead(GPIO_OPEN_2)) {
        digitalWrite(GPIO_OPEN_2, false);
        pi_open2 = false;
        log_i("Door 2 closed because of timeout");
    }
}


/* check_voltages()
 * 
 * Uses the ADC to check the various system rails.
 */
uint8_t check_voltages() {
    uint16_t v_adc;
    float v;
    uint8_t errors = 0;
    v_adc = analogRead(GPIO_ADC_V3);
    v = (float) v_adc / V3_FACTOR;
    log_d("V3:    %0.1fV [%d]", v, v_adc);
    if (v < V3_LOW) {
        errors |= ERR_V3_LOW;
    } else if (v > V3_HI) {
        errors |= ERR_V3_HI;
    }
    v_adc = analogRead(GPIO_ADC_V5);
    v = (float) v_adc / V5_FACTOR;
    log_d("V5:    %0.1fV [%d]", v, v_adc);
    if (v < V5_LOW) {
        errors |= ERR_V5_LOW;
    } else if (v > V5_HI) {
        errors |= ERR_V5_HI;
    }
    v_adc = analogRead(GPIO_ADC_VBATT);
    v = ((float) v_adc / VBATT_FACTOR) + 0.5;  // 0.5 accounts for voltage drop across diode
    log_d("VBATT: %0.1fV [%d]", v, v_adc);
    if (v < VBATT_LOW) {
        errors |= ERR_VBATT_LOW;
    } else if (v > VBATT_HI) {
        errors |= ERR_VBATT_HI;
    }
    v_adc = analogRead(GPIO_ADC_VIN);
    v = ((float) v_adc / VIN_FACTOR) + 0.5;  // 0.5 accounts for voltage drop across diode
    log_d("VIN:   %0.1fV [%d]", v, v_adc);
    if (v < VIN_LOW) {
        errors |= ERR_VIN_LOW;
    } else if (v > VIN_HI) {
        errors |= ERR_VIN_HI;
    }
    return errors;
} 

void monitorTask(void * pvParameters) {
    unsigned long loop_counter = 0;
    const long loop_delay = 1000 / LOOP_FREQ;
    unsigned long loop_start = millis();
    long delay_for;
    uint8_t errors;

    /* Configure I2C Character Display */
    setup_i2c_disp();
    display.setCursor(0,0);
    display.print("rLab Door Controller");

    /* Configure Neopixels */
    /*
    CRGB npx1[NPX_NUM_LEDS_1];
    CRGB npx2[NPX_NUM_LEDS_2];
    FastLED.addLeds<NEOPIXEL, GPIO_NPX_1>(npx1, NPX_NUM_LEDS_1);
    FastLED.addLeds<NEOPIXEL, GPIO_NPX_2>(npx2, NPX_NUM_LEDS_2);
    FastLED.clear(true);
    */

    /* Main loop */
    for(;;) {
        /* Run every loop */
        loop_start = millis();
        check_door_state();

        /* Run every 10 loops */
        if (loop_counter % 10 == 0) {
            /* Check the Tamper input */
            if (digitalRead(GPIO_TAMPER)) {
                if (!tamper) {
                    /* This has just happened */
                    tamper = true;
                    log_w("TAMPER - The enclosure is open!");
                } 
            } else {
                if (tamper) {
                    tamper = false;
                    log_i("TAMPER - The enclosure is closed");
                }
            }

            /* Neopixels */
            /*
            fadeToBlackBy( npx1, NPX_NUM_LEDS_1, 128);
            npx1[(loop_counter / LOOP_FREQ) % NPX_NUM_LEDS_1] = CRGB::White;
            FastLED.show();
            */
        }

        /* Run once per second */
        if (loop_counter % LOOP_FREQ == 0) {
            /* Display */
            display.setCursor(0,0);
            display.print("rLab Door Controller");

            /* Check the system voltages */
            errors = check_voltages();
            if(errors) {
                /* One of the voltages is out of limits */
                log_i("At least one of the system voltages is out of limits");
                if ((errors & ERR_VIN_LOW) && !input_power_lost) {
                    input_power_lost = true;
                    log_e("Input power lost or low");
                    /* Display Power message */
                    display.setCursor(0,1);
                    display.print("Power Off - Batt OK ");
                }
                if ((errors & ERR_VBATT_LOW) && !battery_low) {
                    battery_low = true;
                    log_w("Battery voltage is low, door may go offline shortly...");
                    /* Display Power message */
                    display.setCursor(0,1);
                    display.print("Power Off - Batt Low");
                }
            } else {
                /* No errors, so clear the flags */
                if (input_power_lost || battery_low) {
                    log_i("System power is OK");
                    input_power_lost = false;
                    battery_low = false;
                } 
                
                /* Display Power OK message */
                display.setCursor(0,1);
                display.print("      Power OK      ");
            }

            /* Show uptime */
            display.setCursor(0,3);
            uptime::calculateUptime();
            display.printf("Up: %03lud %02luh %02lum %02lus",
                            uptime::getDays(),
                            uptime::getHours(),
                            uptime::getMinutes(),
                            uptime::getSeconds()
            );
        }

        /* Run once per hour */
        if (loop_counter % (LOOP_FREQ * 3600) == 0) {

        }


        /* Work out how long we have to sleep for */
        delay_for = (loop_delay - (millis() - loop_start));
        delay_for = delay_for < 0 ? 0 : delay_for;
        //log_d("Monitor loop sleeping for %dms", delay_for);
        delay(delay_for);
        loop_counter++;
    }
}