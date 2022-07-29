#ifndef FEATURES_H
#define FEATURES_H

#include <Arduino.h>


/* Functions */
void load_prefs();
void show_features(HardwareSerial * console);
int configure_wifi(char * ssid, char * passwd);
void start_wifi();
void setup_gpio();

#endif