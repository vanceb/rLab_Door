#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>

#include <conf.h>
#include <features.h>
#include <console_ui.h>
#include <Preferences.h>
#include <pi_control.h>
#include <monitor.h>


/* Global variables */
TaskHandle_t consoleTaskHandle = NULL;
TaskHandle_t monitorTaskHandle = NULL;
Pushover pushover;
Preferences prefs;


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
  prefs.begin(PREFS_NS);
  load_prefs();
  //pushover.configure();
  Serial.println(show_features());
  
  /* Bring up wifi if enabled and configured 
   * Otherwise this does nothing...
   */
  start_wifi();

  /* Configure GPIOs */
  setup_gpio();

  /* Check the Raspberry Pi */
  Pi rpi;
  rpi.begin(
    GPIO_PI_POWERED,
    GPIO_PI_HEARTBEAT,
    GPIO_PI_OPEN_CMD_1,
    GPIO_PI_OPEN_CMD_2
  );
  delay(1000);  // give the ISR chance to fire
  if(rpi.is_connected()) {
    log_i("Detected Raspberry Pi");
  } else {
    log_w("No Raspberry Pi detected");
  }
  if(rpi.is_alive()) {
    log_i("Detected Pi Heartbeat - service running");
  } else {
    log_w("No Heartbeat detected from the Pi - check if service is running");
  }

  /* Configure I2C for Character Display */
  setup_i2c_disp();

  /* Install log manager */
  // Doesn't work under Arduino framework!
  // Log level set in platformio.ini build_flags
  //esp_log_set_vprintf(&_log_vprintf);
  //esp_log_level_set("*", ESP_LOG_INFO);

  pushover.send("rLab Door Booting", "The rLabDoor controller is booting up", -1);

  /* Start the FreeRTOS tasks */
  xTaskCreate(consoleTask, "Console Task", 10000, (void*) &Serial, 8, &consoleTaskHandle);
  xTaskCreate(monitorTask, "Monitor Task", 5000, NULL, 16, &monitorTaskHandle);
}

void loop() {
  delay(15000);
  console_hwm = uxTaskGetStackHighWaterMark(consoleTaskHandle);
  if (console_hwm != prev_console_hwm) {
    prev_console_hwm = console_hwm;
    log_i("Console Stack: %d bytes remaining", console_hwm);
  }
}