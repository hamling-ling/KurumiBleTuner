#ifndef _SERIALCONTROLLER_H_
#define _SERIALCONTROLLER_H_

#include <inttypes.h>

class HardwareSerial;

class SerialController
{
public:
    SerialController(HardwareSerial& serial);
    ~SerialController();
    
    void SetTimeout(uint32_t ms);
    void PrintLn(const char* str);
    void ReadLine(char* buf, int buf_len);
    
    const char* WaitForOneOfWords(const char** words, int wordNum, int sleep, int retry);
    bool WaitForWord(const char* word, int sleep, int retry);
    void Purge();
    
private:
    HardwareSerial& _serial;
};

#endif
