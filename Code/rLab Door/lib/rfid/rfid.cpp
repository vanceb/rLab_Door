#include <rfid.h>
#include <Arduino.h>
#include <NfcAdapter.h>

bool rfid_open1 = false;
bool rfid_open2 = false;

void rfidTask(void * pvParameters) {
    /* Cast the incoming parameter to a serial port */
    NfcAdapter * nfc;
    nfc = (NfcAdapter *) pvParameters;
    nfc->begin();

    for(;;) {
        if (nfc->tagPresent())
        {
            NfcTag tag = nfc->read();
            tag.print();
        }
        delay(1000);
    }
}