#include <conf.h>
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
    msg_queue = NULL;
}

Pushover::~Pushover()
{

}

int Pushover::configure(const char * user_key, const char * api_key, const char * url) { 
    memset(po_user_key, 0, PUSHOVER_USER_KEY_MAX_LEN);
    memset(po_api_key,  0, PUSHOVER_API_KEY_MAX_LEN);
    memset(po_api_url,  0, PUSHOVER_URL_MAX_LEN);
    
    msg_queue = xQueueCreate(PUSHOVER_MESSAGE_QUEUE_LEN, sizeof(Message));

    /* Store supplied credentials into flash */
    if (strlen(user_key) > 0 && strlen(api_key) > 0) {
        prefs.putString(PREFS_PO_USER_KEY, user_key);
        prefs.putString(PREFS_PO_API_KEY,  api_key);
        if (strlen(url) > 0) {
            /* Use the url provided */
            prefs.putString(PREFS_PO_URL, url);
        } else {
            /* Use the default */
            prefs.putString(PREFS_PO_URL, PUSHOVER_DEFAULT_URL);
        }
        log_i("Saved pushover credentials");
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

    log_d("Pushover: %s %s %s", po_user_key, po_api_key, po_api_url);
    /* Sanity check */
    if (strlen(po_user_key) > 0 &&
        strlen(po_api_key)  > 0 &&
        strlen(po_api_url)  > 0) {
            configured = true;
    } else {
        configured = false;
    }
    return configured;
}

int Pushover::configure() {
    char empty[1] = { 0 }; 
    return configure(empty, empty, empty);
}

bool Pushover::is_configured() {
    return configured;
}

int Pushover::send(const char * title, const char * msg, int priority) {
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
            char p[3];
            if (priority < -2 || priority > 2) {
                log_w("Priority out of range (%d), must be -2 <= priority <= 2", priority);
                itoa(0, p, 10);
            } else {
                itoa(priority, p, 10);
            }
            
            int max_msg_len = PUSHOVER_MAX_PAYLOAD_LEN - (93 + strlen(title));
            char payload[PUSHOVER_MAX_PAYLOAD_LEN];
            strcpy(payload, "token=");      // 6 bytes
            strcat(payload, po_api_key);    // 30 bytes
            strcat(payload, "&user=");      // 6 bytes
            strcat(payload, po_user_key);   // 30 bytes
            strcat(payload, "&title=");     // 7 bytes
            strcat(payload, title);         // Unknown
            strcat(payload, "&priority=");  // 10 bytes
            strcat(payload, p);             // 2 bytes
            strcat(payload, "&message=");
            strncat(payload, msg, max_msg_len);

            log_d("%s", payload);

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

int Pushover::send(Message * msg) {
    return send(msg->title, msg->body, msg->priority);
}

QueueHandle_t Pushover::getQueue() {
    return msg_queue;
}


void pushoverTask( void * pvParameters) {

    /* Cast the incoming parameter to a Pushover object */
    Pushover * po;
    po = (Pushover*) pvParameters;
    /* Then get its queue */
    QueueHandle_t queue;
    queue = po->getQueue();
    Message msg;

    for (;;) {
        /* Check queue for message */
        if (xQueueReceive(queue, (void*) &msg, 0)) {
            log_d("Pushover sender got msg: %s", msg.title);
            po->send(&msg);
        }
        delay(100);
    }



}