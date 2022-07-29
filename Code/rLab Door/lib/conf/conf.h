#ifndef PROJ_CONFIG_H
#define PROJ_CONFIG_H

#include <Preferences.h>

#include <pushover.h>

/* Preferences stored (Flash) */
#define PREFS_NS            "rlabDoor"      // Preferences namespace
#define PREFS_HARDWARE_KEY  "hw_enabled"    // Key for soring hardware enabled
#define PREFS_WIFI_SSID_KEY "wifi_ssid"     // Key for storing ssid
#define PREFS_WIFI_PWD_KEY  "wifi_passwd"   // Key for storing wifi passwd

/* Global variables for single instances used across the code */
extern Pushover pushover;
extern Preferences prefs;

#endif