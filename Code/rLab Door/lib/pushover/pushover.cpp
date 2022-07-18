#include <pushover.h>

#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>

char po_user_key[PUSHOVER_USER_KEY_MAX_LEN] = {0};
char po_api_key [PUSHOVER_API_KEY_MAX_LEN] = {0};
char po_api_url [PUSHOVER_URL_MAX_LEN] = {0};

Pushover::Pushover()
{
    configured = false;
}

Pushover::~Pushover()
{
}

int Pushover::configure(char * user_key, char * api_key, char * url) { 
    memset(po_user_key, 0, PUSHOVER_USER_KEY_MAX_LEN);
    memset(po_api_key,  0, PUSHOVER_API_KEY_MAX_LEN);
    memset(po_api_url,  0, PUSHOVER_URL_MAX_LEN);
    
    Preferences prefs;
//    prefs.begin(PREFS_PO_NAMESPACE);

    /* Store supplied credentials into flash */
    if (user_key != NULL && api_key != NULL) {
        prefs.putString(PREFS_PO_USER_KEY, user_key);
        prefs.putString(PREFS_PO_API_KEY,  api_key);
        if (url == NULL) {
            /* Use the default */
            prefs.putString(PREFS_PO_URL, PUSHOVER_DEFAULT_URL);
        } else {
            /* Use the url provided */
            prefs.putString(PREFS_PO_URL, url);
        }
    } 
    
    /* Try and grab the creds from flash */
    if (prefs.isKey(PREFS_PO_USER_KEY) && prefs.isKey(PREFS_PO_API_KEY)) {
        /* Get the stored keys */
        prefs.getString(PREFS_PO_USER_KEY, po_user_key, PUSHOVER_USER_KEY_MAX_LEN);
        prefs.getString(PREFS_PO_API_KEY, po_api_key, PUSHOVER_API_KEY_MAX_LEN);
        prefs.getString(PREFS_PO_URL, po_api_url, PUSHOVER_URL_MAX_LEN);
    } else {
        log_e("Unable to retreive pushover preferences from flash");
    }
//    prefs.end();

    /* Sanity check */
    if (strlen(po_user_key) != 0 &&
        strlen(po_api_key)  != 0 &&
        strlen(po_api_url)  != 0) {
            configured = true;
    } else {
        configured = false;
    }
    return configured;
}

bool Pushover::is_configured() {
    return configured;
}

int Pushover::send(char * title, char * msg, int priority) {
    int timeout =  5;
    int httpCode = 0;
    if(configured) {
        while((WiFi.status() != WL_CONNECTED) && timeout > 0) {
            delay(1000);
            timeout--;
        }
        if(timeout < 0) {
            log_w("Failed to send message to pushover, wifi down!");
            return -2;
        } else {
            /* Send the message */
            char p[2];
            if (priority < 0 || priority > 10) {
                itoa(1, p, 10);
            } else {
                itoa(priority, p, 10);
            }
            int max_msg_len = PUSHOVER_MAX_PAYLOAD_LEN - (92 + strlen(title));
            char payload[PUSHOVER_MAX_PAYLOAD_LEN];
            strcat(payload, "token=");      // 6 bytes
            strcat(payload, po_api_key);    // 30 bytes
            strcat(payload, "&user=");      // 6 bytes
            strcat(payload, po_user_key);   // 30 bytes
            strcat(payload, "&title=");     // 7 bytes
            strcat(payload, title);         // Unknown
            strcat(payload, "&priority=");  // 10 bytes
            strcat(payload, p);             // 1 byte
            strcat(payload, "&message=");
            strncat(payload, msg, max_msg_len);

            HTTPClient https;
            https.begin(po_api_url, DIGICERT_ROOT_CA);
            https.addHeader("Content-Type", "application/x-www-form-urlencoded");
            httpCode = https.POST(payload);

        }
        return httpCode;
    }
    log_w("Attempt to send message to pushover, but it has not been configured");
    return -1;
}