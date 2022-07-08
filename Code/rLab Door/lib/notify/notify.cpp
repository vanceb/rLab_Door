#include <notify.h>

#include <stdarg.h>

Notify::Notify()
{
    num_tgts = 0;
}

Notify::~Notify()
{
}

void Notify::add(Notify_Target * tgt) {
    targets[num_tgts] = tgt;
    num_tgts++;
}

void Notify::msg(uint8_t level, const char * fmt, va_list args) {
    char message[NOTIFY_MAX_MSG_LEN];
    vsnprintf(message, NOTIFY_MAX_MSG_LEN, fmt, args);
    for (int i=0; i<num_tgts; i++) {
        targets[i]->notify(level, message);
    }    
}

void Notify::error(const char * fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);
    msg(NOTIFY_ERROR, fmt, argptr);
    va_end(argptr);
}

void Notify::warn(const char * fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);
    msg(NOTIFY_WARN, fmt, argptr);
    va_end(argptr);
}

void Notify::info(const char * fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);
    msg(NOTIFY_INFO, fmt, argptr);
    va_end(argptr);
}

void Notify::debug(const char * fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);
    msg(NOTIFY_DEBUG, fmt, argptr);
    va_end(argptr);
}

void Notify::verbose(const char * fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);
    msg(NOTIFY_VERBOSE, fmt, argptr);
    va_end(argptr);
}


Notify_Target::Notify_Target(uint8_t level) {
    set_level(level);
}

int Notify_Target::set_level(uint8_t level) {
    this->level = level;
    return this->level;
}

Notify_Console::Notify_Console(uint8_t level, HardwareSerial * serial) 
: Notify_Target(level) {
    this->serial = serial;
}

void Notify_Console::notify(uint8_t level, char * msg) {
    if (level <= this->level) {
        serial->print(msg);
    }
}

Notify_Log::Notify_Log(uint8_t level) : Notify_Target(level) {

}

void Notify_Log::notify(uint8_t level, char * msg) {
    if (level <= this->level) {
        switch (level)
        {
        case 0x01:
            log_e("%s", msg);
            break;
        case 0x02:
            log_w("%s", msg);
            break;
        case 0x03:
            log_i("%s", msg);
            break;
        case 0x04:
            log_d("%s", msg);
            break;
        case 0x05:
            log_v("%s", msg);
            break;
        
        default:
            break;
        }
    }
}