#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>

#include <features.h>

uint32_t enabled;
uint32_t configured = 0;
uint32_t status = 0;

char msg[MAX_MSG_LEN];

char wifi_ssid[WIFI_SSID_MAX_LEN] = {0};
char wifi_passwd[WIFI_PASSWD_MAX_LEN] = {0};

uint32_t set(uint32_t * bitfield, uint32_t feature) {
    *bitfield |= feature;
    return *bitfield;
}

uint32_t clear(uint32_t * bitfield, uint32_t feature) {
    *bitfield &= !feature;
    return *bitfield;
}

void load_prefs()
{
    Preferences prefs;
    prefs.begin(PREFS_NS);

    /* Check for hardware features enabled config setting */
    if (!prefs.isKey(PREFS_HARDWARE_KEY))
    {
        log_w("No hardware enable config stored, enabling all hardware features");
        prefs.putUInt(PREFS_HARDWARE_KEY, 0xFFFFFFFF);
    }
    /* Read values from flash */
    enabled = prefs.getUInt(PREFS_HARDWARE_KEY);

    /* Check whether enabled features are configured */
    /* Wifi */
    if (prefs.isKey(PREFS_WIFI_SSID_KEY) && prefs.isKey(PREFS_WIFI_PWD_KEY))
    {
        configured |= FEATURE_WIFI;
        /* Get the stored ssid and passwd */
        prefs.getString(PREFS_WIFI_SSID_KEY, wifi_ssid, WIFI_SSID_MAX_LEN);
        prefs.getString(PREFS_WIFI_PWD_KEY, wifi_passwd, WIFI_PASSWD_MAX_LEN);
    }

    prefs.end();
}

char *show_features()
{
    /* List enabled features */
    memset(msg, 0, MAX_MSG_LEN); // Clear the response message
    snprintf(msg, MAX_MSG_LEN, \
        "Wifi:               %s, %s\n"
        "Pi:                 %s\n"
        "Character display:  %s\n"
        "Neopixel 1:         %s\n"
        "Neopixel 2:         %s\n"
        "NFC reader:         %s, %s\n"
        "Keypad:             %s\n"
        "Open 2 output:      %s\n",
        (enabled    & FEATURE_WIFI)    ? "enabled" : "disabled",
        (configured & FEATURE_WIFI)    ? "configured" : "not configured",
        (enabled    & FEATURE_PI)      ? "enabled" : "disabled",
        (enabled    & FEATURE_DISP)    ? "enabled" : "disabled",
        (enabled    & FEATURE_NPX_1)   ? "enabled" : "disabled",
        (enabled    & FEATURE_NPX_2)   ? "enabled" : "disabled",
        (enabled    & FEATURE_NFC)     ? "enabled" : "disabled",
        (configured & FEATURE_NFC)     ? "configured" : "not configured",
        (enabled    & FEATURE_KEYPAD)  ? "enabled" : "disabled",
        (enabled    & FEATURE_OPEN_2)  ? "enabled" : "disabled"
    );
    return msg;
}

int configure_wifi(char * ssid, char * passwd) {
    set(&enabled, FEATURE_WIFI);
    WiFi.begin(ssid, passwd);
    int seconds = 10;
    while (seconds > 0 && WiFi.status() != WL_CONNECTED) {
        seconds--;
        delay(1000);
    } 
    if (WiFi.status() == WL_CONNECTED) {
        /* Store SSID and password */
        Preferences prefs;
        prefs.begin(PREFS_NS);
        prefs.putString(PREFS_WIFI_SSID_KEY, ssid);
        prefs.putString(PREFS_WIFI_PWD_KEY,  passwd);
        prefs.end();
        /* reload preferences from flash */
        load_prefs();
        set(&configured, FEATURE_WIFI);
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
            WiFi.begin(wifi_ssid, wifi_passwd);
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

int setup_i2c_disp() {
    if (enabled & FEATURE_DISP) {   
        Wire.begin(GPIO_SDA_DISP, GPIO_SCL_DISP);
        /* Check to see that we have a display */
        Wire.beginTransmission(DISP_ADDR);
        if (Wire.endTransmission() == 0) {
            log_i("Character display found at i2c 0x%02X", DISP_ADDR);
            return true;
        } else {
            log_w("Character display NOT found at i2c 0x%02X", DISP_ADDR);
            return false;
        }
    }
}