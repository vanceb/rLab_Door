#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <ArduinoNvs.h>

#include <config.h>
#include <console_ui.h>
#include <log_manager.h>
#include <notify.h>

/* Global variables */
TaskHandle_t consoleTaskHandle = NULL;
UBaseType_t console_hwm, prev_console_hwm;

/* Additional Serial Ports 
 * See https://quadmeup.com/arduino-esp32-and-3-hardware-serial-ports/?utm_source=pocket_mylist
 */

HardwareSerial Pi_Serial(1);
HardwareSerial NFC_Serial(2);

/* Hardware status */
bool display_connected;
bool pi_connected;

#define TAG "rLabDoor"

void setup() {
  /* Start access to NVS for config storage/retreival */
  NVS.begin();

  /* Configure UARTs */
  Serial.begin    (115200, SERIAL_8N1, GPIO_RXDI_PROG, GPIO_TXDO_PROG);
  Pi_Serial.begin (115200, SERIAL_8N1, GPIO_RXDI_PI, GPIO_TXDO_PI);
  NFC_Serial.begin(115200, SERIAL_8N1, GPIO_RXDI_NFC, GPIO_TXDO_NFC);

  /* Setup notification and logging */
  Notify * notify = new Notify();
  Notify_Console * console = new Notify_Console(NOTIFY_INFO, &Serial);
  notify->add(console);
  Notify_Log * log = new Notify_Log(NOTIFY_INFO);
  notify->add(log);

  int i = 123;
  notify->info("Logging started! [%d]\n", i);

  /* Configure GPIOs */
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

  /* Configure I2C for Character Display */
  Wire.begin(GPIO_SDA_DISP, GPIO_SCL_DISP);
  /* Check to see that we have a display */
  Wire.begin(DISP_ADDR);
  if (Wire.endTransmission() == 0) {
    ESP_LOGI(TAG, "Character display found at i2c 0x%02X", DISP_ADDR);
    display_connected = true;
  } else {
    ESP_LOGW(TAG, "Character display NOT found at i2c 0x%02X", DISP_ADDR);
    display_connected = false;
  }


  /* Install log manager */
  // Doesn't work under Arduino framework!
  // Log level set in platformio.ini build_flags
  //esp_log_set_vprintf(&_log_vprintf);
  //esp_log_level_set("*", ESP_LOG_INFO);

  /* Start the FreeRTOS tasks */
  xTaskCreate(consoleTask, "Console Task", 2000, NULL, 8, &consoleTaskHandle);
}

void loop() {
  delay(15000);
  console_hwm = uxTaskGetStackHighWaterMark(consoleTaskHandle);
  if (console_hwm != prev_console_hwm) {
    prev_console_hwm = console_hwm;
    ESP_LOGI(TAG, "Console Stack: %d", console_hwm);
  }
}