#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <Preferences.h>

#include <PN532_HSU.h>
#include <PN532.h>
#include <NfcAdapter.h>

#include <conf.h>
#include <hardware.h>
#include <console_ui.h>
#include <pi_control.h>
#include <monitor.h>
#include <rfid.h>


/* Global variables */
TaskHandle_t consoleTaskHandle  = NULL;
TaskHandle_t monitorTaskHandle  = NULL;
TaskHandle_t pushoverTaskHandle = NULL;
TaskHandle_t rfidTaskHandle     = NULL;

/* Stack high water marks - checked in loop() */
UBaseType_t console_hwm;
UBaseType_t prev_console_hwm = 0;
UBaseType_t monitor_hwm;
UBaseType_t prev_monitor_hwm = 0;
UBaseType_t pushover_hwm;
UBaseType_t prev_pushover_hwm = 0;
UBaseType_t rfid_hwm;
UBaseType_t prev_rfid_hwm = 0;

Pushover pushover;
Preferences prefs;


/* Additional Serial Ports 
 * See https://quadmeup.com/arduino-esp32-and-3-hardware-serial-ports/?utm_source=pocket_mylist
 */

HardwareSerial Pi_Serial(1);
HardwareSerial NFC_Serial(2);

void setup () {
  /* Configure UARTs */
  Serial.begin    (115200, SERIAL_8N1, GPIO_RXDI_PROG, GPIO_TXDO_PROG);
  Pi_Serial.begin (115200, SERIAL_8N1, GPIO_RXDI_PI, GPIO_TXDO_PI);
  NFC_Serial.begin(115200, SERIAL_8N1, GPIO_RXDI_NFC, GPIO_TXDO_NFC);

  /* Load preferences from flash */
  prefs.begin(PREFS_NS);
  load_prefs();
  //pushover.configure();
  show_features(&Serial);
  
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

  /* Install log manager */
  // Doesn't work under Arduino framework!
  // Log level set in platformio.ini build_flags
  //esp_log_set_vprintf(&_log_vprintf);
  //esp_log_level_set("*", ESP_LOG_INFO);

  pushover.send("rLab Door Booting", "The rLabDoor controller is booting up", -1);
  
  QueueHandle_t po_queue = pushover.getQueue();
  Message msg;
  strlcpy(msg.title, "Test from rLabDoor", PUSHOVER_MAX_TITLE_LEN);
  strlcpy(msg.body, "This is the body", PUSHOVER_MAX_MESSAGE_LEN);
  msg.priority = -1;
  xQueueSendToBack(po_queue, &msg, 1);

  /* Configure RFID */
  /* Commented out below as nfc.begin() in the rfidTask
   * interferes with display and Pi 
   */
/*
  PN532_HSU pn532_hsu(NFC_Serial);
  NfcAdapter nfc = NfcAdapter(pn532_hsu);
*/

  /* Start the FreeRTOS tasks */
  xTaskCreate(consoleTask, "Console Task", 5000, (void*) &Serial, 8, &consoleTaskHandle);
  xTaskCreate(monitorTask, "Monitor Task", 5000, NULL, 16, &monitorTaskHandle);
  xTaskCreate(pushoverTask, "Pushover Task", 8000, (void*) &pushover, 8, &pushoverTaskHandle);
//  xTaskCreate(rfidTask, "RFID Task", 5000, (void*) &nfc, 8, &rfidTaskHandle);
}

void loop() {
  delay(15000);
  console_hwm = uxTaskGetStackHighWaterMark(consoleTaskHandle);
  if (console_hwm != prev_console_hwm) {
    prev_console_hwm = console_hwm;
    log_i("Console Stack: %d bytes remaining", console_hwm);
  }
  
  monitor_hwm = uxTaskGetStackHighWaterMark(monitorTaskHandle);
  if (monitor_hwm != prev_monitor_hwm) {
    prev_monitor_hwm = monitor_hwm;
    log_i("Monitor Stack: %d bytes remaining", monitor_hwm);
  }

  pushover_hwm = uxTaskGetStackHighWaterMark(pushoverTaskHandle);
  if (pushover_hwm != prev_pushover_hwm) {
    prev_pushover_hwm = pushover_hwm;
    log_i("Pushover Stack: %d bytes remaining", pushover_hwm);
  }

  rfid_hwm = uxTaskGetStackHighWaterMark(rfidTaskHandle);
  if (rfid_hwm != prev_rfid_hwm) {
    prev_rfid_hwm = rfid_hwm;
    log_i("RFID Stack: %d bytes remaining", rfid_hwm);
  }

}