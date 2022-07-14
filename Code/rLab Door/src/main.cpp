#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>

#include <features.h>
#include <console_ui.h>
#include <log_manager.h>
#include <notify.h>

/* Global variables */
TaskHandle_t consoleTaskHandle = NULL;

/* Additional Serial Ports 
 * See https://quadmeup.com/arduino-esp32-and-3-hardware-serial-ports/?utm_source=pocket_mylist
 */

HardwareSerial Pi_Serial(1);
HardwareSerial NFC_Serial(2);

UBaseType_t console_hwm;
UBaseType_t prev_console_hwm = 0;

void setup () {
  /* Configure UARTs */
  Serial.begin    (115200, SERIAL_8N1, GPIO_RXDI_PROG, GPIO_TXDO_PROG);
  Pi_Serial.begin (115200, SERIAL_8N1, GPIO_RXDI_PI, GPIO_TXDO_PI);
  NFC_Serial.begin(115200, SERIAL_8N1, GPIO_RXDI_NFC, GPIO_TXDO_NFC);

  /* Load preferences from flash */
  load_prefs();
  Serial.println(show_features());
  
  /* Bring up wifi if enabled and configured 
   * Otherwise this does nothing...
   */
  start_wifi();

  /* Setup notification and logging */
  //Notify * notify = new Notify();
  //Notify_Console * console = new Notify_Console(NOTIFY_INFO, &Serial);
  //notify->add(console);
  //Notify_Log * log = new Notify_Log(NOTIFY_INFO);
  //notify->add(log);
  //notify->info("Logging started!");

  /* Configure GPIOs */
  setup_gpio();

  /* Configure I2C for Character Display */
  setup_i2c_disp();

  /* Install log manager */
  // Doesn't work under Arduino framework!
  // Log level set in platformio.ini build_flags
  //esp_log_set_vprintf(&_log_vprintf);
  //esp_log_level_set("*", ESP_LOG_INFO);

  /* Start the FreeRTOS tasks */
  xTaskCreate(consoleTask, "Console Task", 3000, (void*) &Serial, 8, &consoleTaskHandle);
}

void loop() {
  delay(15000);
  console_hwm = uxTaskGetStackHighWaterMark(consoleTaskHandle);
  if (console_hwm != prev_console_hwm) {
    prev_console_hwm = console_hwm;
    log_i("Console Stack: %d bytes remaining", console_hwm);
  }
}