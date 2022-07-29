#ifndef PROJ_CONFIG_H
#define PROJ_CONFIG_H

#include <Preferences.h>

#include <pushover.h>


/* Max variable string lengths */
/* Wifi */
#define WIFI_SSID_MAX_LEN   16
#define WIFI_PASSWD_MAX_LEN 32

/* Preferences stored (Flash) */
#define PREFS_NS            "rlabDoor"      // Preferences namespace
#define PREFS_HARDWARE_KEY  "hw_enabled"    // Key for soring hardware enabled
#define PREFS_WIFI_SSID_KEY "wifi_ssid"     // Key for storing ssid
#define PREFS_WIFI_PWD_KEY  "wifi_passwd"   // Key for storing wifi passwd

/* Global variables used to hold values stored in flash
 * so we can read the flash only at startup or on config change
 */
extern char wifi_ssid  [WIFI_SSID_MAX_LEN];
extern char wifi_passwd[WIFI_PASSWD_MAX_LEN];

/* Character buffer for messages */
/*
#define MAX_MSG_LEN         512
extern char msg[MAX_MSG_LEN];
*/

/* Global variables for single instances used across the code */
extern Pushover pushover;
extern Preferences prefs;

#endif