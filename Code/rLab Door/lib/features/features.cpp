#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>

#include <hardware.h>
#include <features.h>
#include <conf.h>


uint32_t enabled;
uint32_t configured = 0;
uint32_t status = 0;

char wifi_ssid[WIFI_SSID_MAX_LEN] = {0};
char wifi_passwd[WIFI_PASSWD_MAX_LEN] = {0};


void load_prefs()
{
    /* Check for hardware features enabled config setting */
    if (!prefs.isKey(PREFS_HARDWARE_KEY))
    {
        log_w("No hardware enable config stored, enabling all hardware features");
        prefs.putUInt(PREFS_HARDWARE_KEY, 0xFFFFFFFF);
    }
    /* Read values from flash */
    enabled = prefs.getUInt(PREFS_HARDWARE_KEY);
    /* Assume all configured, unless we find they are not
     * Most don't need configuration so this ensures that 
     * they report as configured
     */
    configured = 0xFFFFFFFF;

    /* Check whether enabled features are configured */
    /* Wifi */
    if (prefs.isKey(PREFS_WIFI_SSID_KEY) && prefs.isKey(PREFS_WIFI_PWD_KEY)) {
        /* Get the stored ssid and passwd */
        prefs.getString(PREFS_WIFI_SSID_KEY, wifi_ssid, WIFI_SSID_MAX_LEN);
        prefs.getString(PREFS_WIFI_PWD_KEY, wifi_passwd, WIFI_PASSWD_MAX_LEN);
        configured |= FEATURE_WIFI;
    } else {
        /* Not configured so clear the flag */
        configured &= ~(FEATURE_WIFI);
    }

    /* Check pushover */
    if (!pushover.configure()) {
        configured &= ~(FEATURE_PUSHOVER);
    }
}

void show_features(HardwareSerial * console) 
{
    console -> printf(
        "Wifi:              %s, %s\n"
        "Pi:                %s, %s\n"
        "Character display: %s, %s\n"
        "Neopixel 1:        %s, %s\n"
        "Neopixel 2:        %s, %s\n"
        "NFC reader:        %s, %s\n"
        "Keypad:            %s, %s\n"
        "Open 2 output:     %s, %s\n"
        "Pushover notify:   %s, %s\n",
        (enabled    & FEATURE_WIFI)     ? "enabled" : "disabled",
        (configured & FEATURE_WIFI)     ? "configured" : "not configured",
        (enabled    & FEATURE_PI)       ? "enabled" : "disabled",
        (configured & FEATURE_PI)       ? "configured" : "not configured",
        (enabled    & FEATURE_DISP)     ? "enabled" : "disabled",
        (configured & FEATURE_DISP)     ? "configured" : "not configured",
        (enabled    & FEATURE_NPX_1)    ? "enabled" : "disabled",
        (configured & FEATURE_NPX_1)    ? "configured" : "not configured",
        (enabled    & FEATURE_NPX_2)    ? "enabled" : "disabled",
        (configured & FEATURE_NPX_2)    ? "configured" : "not configured",
        (enabled    & FEATURE_NFC)      ? "enabled" : "disabled",
        (configured & FEATURE_NFC)      ? "configured" : "not configured",
        (enabled    & FEATURE_KEYPAD)   ? "enabled" : "disabled",
        (configured & FEATURE_KEYPAD)   ? "configured" : "not configured",
        (enabled    & FEATURE_OPEN_2)   ? "enabled" : "disabled",
        (configured & FEATURE_OPEN_2)   ? "configured" : "not configured",
        (enabled    & FEATURE_PUSHOVER) ? "enabled" : "disabled",
        (configured & FEATURE_PUSHOVER) ? "configured" : "not configured"
    );
}

int configure_wifi(char * ssid, char * passwd) {
    enabled |= FEATURE_WIFI;
    WiFi.begin(ssid, passwd);
    int seconds = 10;
    while (seconds > 0 && WiFi.status() != WL_CONNECTED) {
        seconds--;
        delay(1000);
    } 
    if (WiFi.status() == WL_CONNECTED) {
        /* Store SSID and password */
        prefs.putString(PREFS_WIFI_SSID_KEY, ssid);
        prefs.putString(PREFS_WIFI_PWD_KEY,  passwd);
        /* reload preferences from flash */
        load_prefs();
        return 1;
    } else {
        return 0;
    }
}

void start_wifi()
{
    /* Start wifi if configured */
    if (enabled & FEATURE_WIFI)
    {
        if (configured & FEATURE_WIFI)
        {
            /* Connect to wifi */
            log_d("Connecting to wifi: %s", wifi_ssid);
            if(!WiFi.begin(wifi_ssid, wifi_passwd)) {
                log_e("Problem starting up wifi - maybe DNS?");
            } else {

            }
            /* Give it up to 10 seconds to join then continue */
            int seconds = 10;
            while (seconds > 0 && WiFi.status() != WL_CONNECTED)
            {
                seconds--;
                delay(1000);
            }
            if (WiFi.status() == WL_CONNECTED)
            {
                log_i("Connected to wifi ssid: %s", wifi_ssid);
            }
            else
            {
                log_w("Didn't connect to wifi ssid %s within 10 seconds", wifi_ssid);
            }
        }
        else
        {
            log_i("Wifi not configured");
        }
    }
    else
    {
        log_i("Wifi not enabled");
    }
}

void setup_gpio() {
  pinMode(GPIO_PI_HEARTBEAT,  INPUT);
  pinMode(GPIO_PI_POWERED,    INPUT);
  pinMode(GPIO_PI_OPEN_CMD_1, INPUT);
  pinMode(GPIO_PI_OPEN_CMD_2, INPUT);

  pinMode(GPIO_TAMPER,        INPUT);
  pinMode(GPIO_ADC_VIN,       INPUT);
  pinMode(GPIO_ADC_VBATT,     INPUT);
  pinMode(GPIO_ADC_V5,        INPUT);
  pinMode(GPIO_ADC_V3,        INPUT);

  pinMode(GPIO_WGD0,          INPUT);
  pinMode(GPIO_WGD1,          INPUT);
  
  pinMode(GPIO_NPX_1,         OUTPUT);
  pinMode(GPIO_NPX_2,         OUTPUT);

  pinMode(GPIO_OPEN_1,        OUTPUT);
  pinMode(GPIO_OPEN_2,        OUTPUT);
}
