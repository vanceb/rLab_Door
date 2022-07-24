#include <Arduino.h>
#include <monitor.h>
#include <pi_control.h>
#include <conf.h>


void monitorTask(void * pvParameters) {
    unsigned long loop_counter = 0;
    const long loop_delay = 1000 / LOOP_FREQ;
    unsigned long loop_start = millis();
    long delay_for;

    bool open1_state = true;
    bool open2_state = true;
    unsigned long open1_changed = 0;
    unsigned long open2_changed = 0;

    /* Main loop */
    for(;;) {
        /* Run every loop */
        loop_start = millis();

        /* Change the state of the door if Pi says so */
        if (open1_state != pi_open1) {
            digitalWrite(GPIO_OPEN_1, pi_open1);
            open1_state = pi_open1;
            open1_changed = millis();
        }

        if (open2_state != pi_open2) {
            digitalWrite(GPIO_OPEN_2, pi_open2);
            open2_state = pi_open2;
            open2_changed = millis();
        }

        /* Check to see whether we should close the door based on timeout */
        if (millis() - open1_changed > DOOR_OPEN_MAX_MS && digitalRead(GPIO_OPEN_1)) {
            digitalWrite(GPIO_OPEN_1, false);
            pi_open1 = false;
        }

        if (millis() - open2_changed > DOOR_OPEN_MAX_MS && digitalRead(GPIO_OPEN_2)) {
            digitalWrite(GPIO_OPEN_2, false);
            pi_open2 = false;
        }


        /* Run every 10 loops */
        if (loop_counter % 10 == 0) {

        }

        /* Run once per second */
        if (loop_counter % LOOP_FREQ == 0) {

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