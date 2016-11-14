#include "StopWatch.h"
#include "Arduino.h"

StopWatch::StopWatch(const char* msg) : _start(micros())
{
    snprintf(_msg, sizeof(_msg), "%s", msg);
    Serial.print(_msg);
    Serial.println(" start");
}

StopWatch::~StopWatch()
{
    unsigned long time = micros() - _start;
    char msg[64] = {'\0'};
    snprintf(msg, sizeof(msg), "%s %lu", _msg, time);
    Serial.println(msg);
}
