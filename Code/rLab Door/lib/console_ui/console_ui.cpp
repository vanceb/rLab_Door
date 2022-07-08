#include "console_ui.h"

#include <WiFi.h>
#include <ArduinoNvs.h>

char rxbuf[BUF_SIZE];
char cmd[BUF_SIZE + 1];
char msg[MAX_MSG_LEN];

void consoleTask(void *pvParameters)
{
    char c;
    uint16_t length = 0;

    while (true)
    {
        if (Serial.available())
        {
            c = Serial.read();
            rxbuf[length++] = c;
            Serial.print(c);
            if (length > BUF_SIZE) {
                Serial.printf("\nInput line too long!  Max %d characters, input cleared!\n", BUF_SIZE);
                length = 0;
            } else {
                if (c == '\n')
                {
                    strlcpy(cmd, (const char *)rxbuf, length);
                    Serial.print(cmdline(cmd));
                    length = 0;
                    Serial.print("> ");
                }
            }
        }
        delay(1);
    }
}


/* The help command */
char *help(char args[MAX_TOKENS][MAX_TOKEN_LEN]) {
    snprintf(msg, MAX_MSG_LEN, \
    "\nValid commands:\n"
    "\thelp - display this help message\n"
    "\techo - echo back the rest of the arguments\n"
    );
    return(msg);
}

/* The echo command */
char * echo(char args[MAX_TOKENS][MAX_TOKEN_LEN]) {
    uint8_t i = 1;
    while (i < MAX_TOKENS && strlen(args[i]) > 0) {
        strcat(msg, args[i]);
        strcat(msg, " ");
        i++;
    }
    strcat(msg, "\n");
    return (msg);
}

/* The wifi setup command */
char * wifi(char args[MAX_TOKENS][MAX_TOKEN_LEN]) {
    /* Check that ssid and passwd have been provided */
    if (strlen(args[1]) == 0 || strlen(args[2]) == 0) {
        strcat(msg, "SSID or password are empty!");
    } else {
        /* Try to connect with provided credentials */

        /* Store SSID and password */
        NVS.setString("wifi_ssid", args[1]);
        NVS.setString("wifi_passwd", args[2]);
        
        strcat(msg, "\n");
    }
    return (msg);
}

/* Console command handler */
char * cmdline(char *request) {
    /* (re)set the variables and buffers */
    uint8_t num_tokens = 0;
    char args[MAX_TOKENS][MAX_TOKEN_LEN] = {""};  // Hold the parsed tokens
    memset(msg, 0, sizeof(msg));    // Clear the response message

    /* Tokenize input up to a maximum of MAX_TOKENS */
    char * t = strtok(request, WHITESPACE);
    while (num_tokens < MAX_TOKENS && t != NULL) {
        strlcpy(args[num_tokens++], t, MAX_TOKEN_LEN);
        t = strtok(NULL, WHITESPACE);
    }

    /* No input */
    if (num_tokens == 0) {
        return(NULL);
    }

    /* Help command */
    if (strcmp(args[0], "help") == 0) {
        return help(args);
    }

    /* echo command */
    if (strcmp(args[0], "echo") == 0) {
        return echo(args);
    }

    /* wifi command */
    if (strcmp(args[0], "wifi") == 0) {
        return wifi(args);
    }

    /* No recognised commands so show the help */
    return help(args);
}
