#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include <Arduino.h>
#include "driver/uart.h"

/* Using UART0 */
#define UART UART0
#define UART_NUM UART_NUM_0

/* UART Rx Buffer */
#define BUF_SIZE (256)
extern char rxbuf[BUF_SIZE];
extern char cmd[BUF_SIZE + 1];

/* Console Commands */
#define MAX_TOKEN_LEN (64)  // longest token length
#define MAX_TOKENS (4)      // Maximum number of tokens to be parsed
#define WHITESPACE "\t \r\n"

void cmdline(HardwareSerial * console, char * request);

void consoleTask( void * pvParameters);

#endif