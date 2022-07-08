#ifndef NOTIFY_H
#define NOTIFY_H

#include <Arduino.h>
#include <stdarg.h>

/* Manage the sending of messages to multiple targets */

/* Levels - Map to standard log levels*/
#define NOTIFY_NONE     0xFF
#define NOTIFY_ERROR    0x01
#define NOTIFY_WARN     0x02
#define NOTIFY_INFO     0x03
#define NOTIFY_DEBUG    0x04
#define NOTIFY_VERBOSE  0x05

/* Targets */
#define NOTIFY_CONSOLE  0x01
#define NOTIFY_PI       0x02
#define NOTIFY_PUSHOVER 0x04
#define NOTIFY_SYSLOG   0x08

/* How many notify targets will we allow */
#define NOTIFY_MAX_TGTS 4

#define NOTIFY_MAX_MSG_LEN  256
#define NOTIFY_MAX_NAME_LEN 16

/* abstract class to be inherited from */
class Notify_Target
{
protected:
    uint8_t level;
public:
    Notify_Target(uint8_t level);
    int set_level(uint8_t level);
    virtual void notify(uint8_t level, char *msg) = 0; // Must be overridden
};

/* Concrete class to send messages to the console */
class Notify_Console : public Notify_Target 
{
private:
    HardwareSerial * serial;
public:
    Notify_Console(uint8_t level, HardwareSerial * serial);
    void notify(uint8_t level, char *msg);
};

/* Concrete class to send messages to the logs */
class Notify_Log : public Notify_Target 
{
private:
public:
    Notify_Log(uint8_t level);
    void notify(uint8_t level, char *msg);
};

class Notify
{
private:
     Notify_Target * targets[NOTIFY_MAX_TGTS];
     uint8_t num_tgts;

public:
    Notify();
    ~Notify();

    void add(Notify_Target * tgt);

    void msg(uint8_t level, const char * fmt, va_list args);
    void error(const char * fmt, ...);
    void warn(const char * fmt, ...);
    void info(const char * fmt, ...);
    void debug(const char * fmt, ...);
    void verbose(const char * fmt, ...);

};

#endif