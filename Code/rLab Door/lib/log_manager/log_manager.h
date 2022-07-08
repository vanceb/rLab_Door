#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <esp32-hal-log.h>

/* Log Manager
 * 
 * Build on top of ESP IDF logging functions to manage log output.
 * Aim is to allow logging to multiple UART outputs
 * and potentially to a character display
 * 
 * Insiration from https://www.esp32.com/viewtopic.php?t=3960
 */


/* _log_vprintf()
 * 
 * Substitute for vprintf() which intercepts logging calls
 * allowing redirection or additional outputs.
 * Needs to be setup in the main code by calling:
 * esp_log_set_vprintf(&_log_vprintf);
 */
int _log_vprintf(const char *fmt, va_list args);

#endif