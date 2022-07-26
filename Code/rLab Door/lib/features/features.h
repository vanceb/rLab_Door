#ifndef FEATURES_H
#define FEATURES_H

#include <Arduino.h>


/* Functions */
void load_prefs();
char * show_features();
int configure_wifi(char * ssid, char * passwd);
void start_wifi();
void setup_gpio();

#endif