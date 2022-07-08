#include <log_manager.h>
#include <stdio.h>

/* _log_vprintf()
 * 
 * Substitute for vprintf() which intercepts logging calls
 * allowing redirection or additional outputs.
 * Needs to be setup in the main code by calling:
 * esp_log_set_vprintf(&_log_vprintf);
 * 
 * This may not work when using the Arduino framework:
 * https://esp32.com/viewtopic.php?t=16825
 */
int _log_vprintf(const char *fmt, va_list args) {
    /* By default send to stdout (Serial) */
    return vprintf(fmt, args);
}