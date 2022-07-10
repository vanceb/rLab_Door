#include <WiFi.h>
#include <Preferences.h>

#include <console_ui.h>
#include <features.h>
#include <notify.h>

char rxbuf[BUF_SIZE];
char cmd[BUF_SIZE + 1];

void consoleTask(void *pvParameters)
{
    /* Cast the incoming parameter to a serial port */
    HardwareSerial * console;
    console = (HardwareSerial*) pvParameters;

    char c;
    uint16_t length = 0;

    while (true)
    {
        if (console->available())
        {
            c = console->read();
            rxbuf[length++] = c;
            console->print(c);
            if (length > BUF_SIZE) {
                console->printf("\nInput line too long!  Max %d characters, input cleared!\n", BUF_SIZE);
                length = 0;
            } else {
                if (c == '\n')
                {
                    strlcpy(cmd, (const char *)rxbuf, length);
                    cmdline(console, cmd);
                    length = 0;
                    console->print("> ");
                }
            }
        }
        delay(1);
    }
}


/* The help command */
void help(HardwareSerial *console, char args[MAX_TOKENS][MAX_TOKEN_LEN]) {
    if (strlen(args[1]) == 0) {
        console->print("\nValid commands:\n");
        console->print("\thelp - display this help message\n");
        console->print("\techo - echo back the rest of the arguments\n");
        console->print("\twifi - setup wifi, or get status\n");
    } else {
        if (strcmp(args[1], "wifi") == 0) {
            console->print("\nwifi <ssid> <password>\n");
            console->print("\tConfigures the wifi to connect to the given ssid using the password\n");
            console->print("\tThe ssid and password will be stored across reboots\n");
            console->print("\nwifi status\n");
            console->print("\tPrints the network status\n");
        }
    }
}

/* The echo command */
void echo(HardwareSerial *console, char args[MAX_TOKENS][MAX_TOKEN_LEN]) {
    uint8_t i = 1;
    while (i < MAX_TOKENS && strlen(args[i]) > 0) {
        console->printf("%s ", args[i]);
        i++;
    }
    console->print("\n");
}

/* The wifi setup command */
void wifi(HardwareSerial *console, char args[MAX_TOKENS][MAX_TOKEN_LEN]) {
    char * ssid   = args[1];
    char * passwd = args[2];
    /* Check that ssid and passwd have been provided */
    if (strlen(ssid) == 0 || strlen(passwd) == 0) {
        console->println("SSID or password are empty!");
    } else {
        /* Try to connect with provided credentials */
        console->printf("Attempting to connect to wifi, ssid %s\n", ssid);
        if (configure_wifi(ssid, passwd)) {
            console->printf("Connected to wifi!\nCredentials saved.");
        } else {
            console->printf("Didn't connect to wifi!\nCredentials not saved.\n");
        }
    }
}

/* Console command handler */
void cmdline(HardwareSerial *console, char *request) {
    /* (re)set the variables and buffers */
    uint8_t num_tokens = 0;
    char args[MAX_TOKENS][MAX_TOKEN_LEN] = {""};  // Hold the parsed tokens

    /* Tokenize input up to a maximum of MAX_TOKENS */
    char * t = strtok(request, WHITESPACE);
    while (num_tokens < MAX_TOKENS && t != NULL) {
        strlcpy(args[num_tokens++], t, MAX_TOKEN_LEN);
        t = strtok(NULL, WHITESPACE);
    }

    /* No input */
    if (num_tokens == 0) {
        return;
    }

    /* echo command */
    if (strcmp(args[0], "echo") == 0) {
        echo(console, args);
    }

    /* wifi command */
    if (strcmp(args[0], "wifi") == 0) {
        wifi(console, args);
    }

    /* No recognised commands so show the help */
    help(console, args);
}
