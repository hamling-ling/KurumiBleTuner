#include "SerialController.h"

#include <Arduino.h>
#include "CommonTool.h"
#include "debug.h"

SerialController::SerialController(HardwareSerial& serial)
: _serial(serial)
{
    _serial.begin(115200);
}

SerialController::~SerialController()
{
}

void SerialController::SetTimeout(uint32_t ms)
{
    _serial.setTimeout(ms);
}

void SerialController::PrintLn(const char* str)
{
    _serial.println(str);
}

void SerialController::ReadLine(char* buf, int buf_len)
{
    memset(buf, 0, sizeof(buf));
    String recv = _serial.readStringUntil('\n');
   	LOG("received:");
    LOG(recv);
    recv.toCharArray(buf, buf_len);
}

const char* SerialController::WaitForOneOfWords(const char** words, int wordNum, int sleep, int retry)
{
    char buf[64] = {0};

    for(int i = 0; i < retry; i++)
    {
        ReadLine(buf, sizeof(buf));
        
        for(int j = 0; j < wordNum; j++) {
            const char* word = words[j];
        
    		if (strncmp(word, buf, strlen(word)) == 0) {
    	        LOG("word received");
    	        return word;
    		}
        }
		LOG("waiting for a words");
		delay(sleep);
    }
    return NULL;
}


void SerialController::Purge()
{
    while(_serial.available() > 0) {
        char c = _serial.read();
        LOG(c);
        delay(10);
    }
}

bool SerialController::WaitForWord(const char* word, int sleep, int retry)
{
    const char* words[] = {word};
    const char* receivedWord = WaitForOneOfWords(words, _countof(words), sleep, retry);
    
    return (receivedWord == word);
}
