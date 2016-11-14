#include <Arduino.h>

#include "Rn4020Controller.h"
#include "StringUtility.h"
#include "SerialController.h"
#include "debug.h"

#define _countof(x) (sizeof(x) / sizeof (x[0]))
#define _strlen(s) (sizeof(s)/sizeof(s[0]))

#define UUID_MIDI_SERVICE		"03B80E5AEDE84B33A7516CE34EC4C700"
#define UUID_CHAR			    "7772E5DB38684112A1A9F2669D106BF3"

#define CMD_FACTORY_RESET		"SF,1"
#define CMD_FUNCTIONS			"SR,80000000"
#define CMD_REBOOT				"R,1"
#define CMD_STARTSCAN			"F"
#define CMD_STOPSCAN			"X"
#define CMD_CONNECT 			"E"
#define CMD_BONDING 			"B"
#define CMD_LISTSERVICES		"LC"

//// template to create CHW,001B,808080007F or CHW,001B,8080C000
#define CHAR_CMD_TEMPLATE       "CHW,001B,"

typedef void (Rn4020Controller::*SendFunc_t)(const SerialCommand_t* pCmd);
typedef bool (Rn4020Controller::*ResultTestFunc_t)();

typedef struct _SerialCommand {
	const char* cmd;
	const int cmd_len;
	const SendFunc_t send_func;
	const ResultTestFunc_t test_func;
	const int timeout;
	const int try_num;
	const int retry_sleep;
} SerialCommand_t;

static const int    kCmdRetry   = 3;
static const char*  kOk         = "AOK";
static const char*  kErr        = "ERR";
static const char*  kReboot     = "Reboot";

const SerialCommand_t Rn4020Controller::kInitCommands[]  = {
	{CMD_FACTORY_RESET,		_strlen(CMD_FACTORY_RESET),		&Rn4020Controller::sendCmd,          &Rn4020Controller::testAOK, 100, 1, 0},
	{CMD_FUNCTIONS,			_strlen(CMD_FUNCTIONS),			&Rn4020Controller::sendCmd,          &Rn4020Controller::testAOK, 100, 1, 0},
	{CMD_REBOOT,			_strlen(CMD_REBOOT),			&Rn4020Controller::sendCmd,          &Rn4020Controller::testReboot, 1000,1, 0},
	{CMD_STARTSCAN,		    _strlen(CMD_STARTSCAN),		    &Rn4020Controller::sendCmd,          &Rn4020Controller::testScanResult, 100, 10, 5000},
	{CMD_STOPSCAN,	        _strlen(CMD_STOPSCAN),	        &Rn4020Controller::sendCmd,          &Rn4020Controller::testAOK, 100, 1, 0},
	{CMD_CONNECT,		    _strlen(CMD_CONNECT),		    &Rn4020Controller::sendConnectCmd,   &Rn4020Controller::testConnectResult, 100, 1, 0},
	{CMD_BONDING,		    _strlen(CMD_BONDING),		    &Rn4020Controller::sendCmd,          &Rn4020Controller::testBondResult, 100, 1, 0},
	{CMD_LISTSERVICES,	    _strlen(CMD_LISTSERVICES),	    &Rn4020Controller::sendCmd,          &Rn4020Controller::testServiceListResult, 100, 1, 0},
};

Rn4020Controller::Rn4020Controller(HardwareSerial& serial)
:
_serial(serial)
{
    memset(m_charHandle, 0, sizeof(m_charHandle));
    memset(m_macAddress, 0, sizeof(m_macAddress));
    memset(m_public, 0, sizeof(m_public));
    strcpy(m_charCmdBuf, CHAR_CMD_TEMPLATE);
}

Rn4020Controller::~Rn4020Controller()
{
}

bool Rn4020Controller::Initialize()
{
    bool result = false;
	
	for(int i = 0; i < _countof(kInitCommands); i++) {
		_serial.Purge();
		
		const SerialCommand_t* pCmd = &kInitCommands[i];
		_serial.SetTimeout(pCmd->timeout);
		
		const char* cmd = pCmd->cmd;
		for(int try_num = 0; try_num < pCmd->try_num; try_num++) {
		
			_serial.Purge();
			
			const SendFunc_t sendfunc = pCmd->send_func;
    		(this->*sendfunc)(pCmd);
    		
    		const ResultTestFunc_t func = pCmd->test_func;
    		if((this->*func)()) {
    		    result = true;
    		    break;
    		}
    		delay(pCmd->retry_sleep);
		}
		
		if(!result) {
		    // return to go over init sequence
		    return false;
		}
	}
	
	strncpy(&m_charCmdBuf[4], m_charHandle, 4);
	
	return result;
}

bool Rn4020Controller::WriteCharacteristic(const char* data, int len)
{
    char cmd[36] = {'0'};
    strncpy(cmd, CHAR_CMD_TEMPLATE, sizeof(cmd));
    strncpy(&cmd[4], m_charHandle, 4);
    int cpylen = min(32-9-1, len);
    strncpy(&cmd[9], data, cpylen);
    cmd[9+cpylen+1] = '\0';
    
    return sendWithAck(cmd);
}

uint32_t Rn4020Controller::Yield()
{
    _serial.Purge();
    return 0;
}

const char* Rn4020Controller::sendWithAck(const char* data)
{
    DEBUG(data);
     _serial.PrintLn(data);
    const char* resp_word[] = { kOk, kErr };
    _serial.WaitForOneOfWords(resp_word, _countof(resp_word), 5, 1);
}

void Rn4020Controller::send(const char* data)
{
    _serial.PrintLn(data);
}

bool Rn4020Controller::parseScanResult(const char* str, char (&mac)[13], char (&pub)[2], char (&dev)[32], char (&uuid)[33])
{
	if (strlen(str) < sizeof(mac) + sizeof(pub) + 1 + sizeof(uuid)) {
		return false;
	}

	int pos = 0;
	copyCommaSeparatedWord(str + pos, mac, sizeof(mac));
	if (strlen(mac) != 12) {
		return false;
	}
	pos += strlen(mac) + 1;
	copyCommaSeparatedWord(str + pos, pub, sizeof(pub));
	if (strlen(pub) != 1) {
		return false;
	}
	pos += strlen(pub) + 1;
	copyCommaSeparatedWord(str + pos, dev, sizeof(dev));
	pos += strlen(dev) + 1;
	copyCommaSeparatedWord(str + pos, uuid, sizeof(uuid));
	if (strlen(uuid) != 32) {
		return false;
	}
	return true;
}

bool Rn4020Controller::parseServiceListResult(const char* str, const char* uuid, char(&handle)[5])
{
	if (strlen(str) < 2 + 40) {
		return false;
	}

	if (str[0] != ' ' || str[1] != ' ') {
		return false;
	}

	int pos = 2;
	char theUuid[33] = { 0 };
	copyCommaSeparatedWord(str + pos, theUuid, sizeof(theUuid));
	if (strcmp(theUuid, uuid) != 0) {
		return false;
	}

	pos += strlen(uuid) + 1;
	copyCommaSeparatedWord(str + pos, handle, sizeof(handle));
	if (strlen(handle) != 4) {
		return false;
	}

	pos += strlen(handle) + 1;
	char prop[3] = { 0 };
	copyCommaSeparatedWord(str + pos, prop, sizeof(prop));
	if (strlen(prop) != 2) {
		return false;
	}
	// better to check bit flag
	if (strcmp(prop, "0E") != 0) {
		return false;
	}
	
	return true;
}

void Rn4020Controller::sendCmd(const SerialCommand_t* pCmd)
{
	LOG(pCmd->cmd);
	_serial.PrintLn(pCmd->cmd);
}

void Rn4020Controller::sendConnectCmd(const SerialCommand_t* pCmd)
{
    char buf[32] = {0};
    buf[0] = pCmd->cmd[0];
    buf[1] = ',';
    buf[2] = m_public[0];
    buf[3] = ',';
    strncpy(buf+4, m_macAddress, 12);
    LOG(buf);
	_serial.PrintLn(buf);
}

bool Rn4020Controller::testAOK()
{
    return _serial.WaitForWord(kOk, 1000, 8);
}

bool Rn4020Controller::testReboot()
{
    _serial.WaitForWord("Reboot", 1000, 16);
    _serial.WaitForWord("CMD", 1000, 16);
}
    
/**
 *  connect command takes sometime. so we need special func
 */
bool Rn4020Controller::testConnectResult()
{
    return _serial.WaitForWord("Connected", 1000, 16);
}

/**
 * test result is something like this
 *  E47118C2BED1,1,UD-BT01,03B80E5AEDE84B33A7516CE34EC4C700,-41
 */
bool Rn4020Controller::testScanResult()
{
    char buf[64] = {0};
    bool result = false;
    // assuming num of services are less than 32
    for(int i = 0; i < 32; i++)
    {
        _serial.ReadLine(buf, sizeof(buf));
        
		char mac[13] = { 0 };
		char pub[2] = { 0 };
		char dev[32] = { 0 };
		char uuid[33] = { 0 };

		if (parseScanResult(buf, mac, pub, dev, uuid)) {
			if(strcmp(uuid, UUID_CHAR) != 0) {
		        result = true;
		        strcpy(m_macAddress, mac);
		        strcpy(m_public, pub);
		        LOG("uuid matched");
		        break;
		    }
		}
    }

    if(!result) {
        // stop scan and prepare for retry
        _serial.PrintLn(CMD_STOPSCAN);
    }

	return result;
}

/**
 * test result is something like this
LC
180A
  2A29,000E,02
  2A24,0010,02
  2A25,0012,02
  2A27,0014,02
  2A26,0016,02
  2A28,0018,02
03B80E5AEDE84B33A7516CE34EC4C700
  7772E5DB38684112A1A9F2669D106BF3,001B,0E
  7772E5DB38684112A1A9F2669D106BF3,001C,10
END
 */
bool Rn4020Controller::testServiceListResult()
{
    char buf[64] = {0};
    bool result = false;
    // assuming 32 lines received
    for(int i = 0; i < 32; i++)
    {
        _serial.ReadLine(buf, sizeof(buf));
        
		if (parseServiceListResult(buf, UUID_CHAR, m_charHandle)) {
		    result = true;
		    LOG("char handle obtained");
			break;
		}
    }

    _serial.WaitForWord("END", 1000, 16);
    
	return result;
}

bool Rn4020Controller::testBondResult()
{
    return _serial.WaitForWord("Bonded", 1000, 16);
}


