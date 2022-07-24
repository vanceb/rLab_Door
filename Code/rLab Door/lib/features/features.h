#ifndef FEATURES_H
#define FEATURES_H

#include <Arduino.h>
#include <Preferences.h>

#include <pushover.h>


extern Pushover pushover;
extern Preferences prefs;

/* Functions */
void load_prefs();
char * show_features();
int configure_wifi(char * ssid, char * passwd);
void start_wifi();
void setup_gpio();
int setup_i2c_disp();

#endif