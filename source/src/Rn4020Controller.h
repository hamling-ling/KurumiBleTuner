#ifndef _RN4020CONTROLLER_H_
#define _RN4020CONTROLLER_H_

#include "SerialController.h"

class HardwareSerial;
typedef struct _SerialCommand SerialCommand_t;

class Rn4020Controller
{
public:
	Rn4020Controller(HardwareSerial& serial);
	~Rn4020Controller();
	
	bool Initialize();
	bool WriteCharacteristic(const char* data, int len);
	uint32_t Yield();
	
	void sendCmd(const SerialCommand_t* pCmd);
	void sendConnectCmd(const SerialCommand_t* pCmd);
	
	bool testAOK();
	bool testReboot();
	bool testConnectResult();
	bool testScanResult();
	bool testServiceListResult();
	bool testBondResult();
	
private:
	SerialController _serial;
	static const SerialCommand_t kInitCommands[];
    char m_public[0];
    char m_charHandle[5];
    char m_macAddress[13];
    char m_charCmdBuf[32];
    
	const char* sendWithAck(const char* data);
	void send(const char* data);
	bool parseScanResult(const char* str, char (&mac)[13], char (&pub)[2], char (&dev)[32], char (&uuid)[33]);
    bool parseServiceListResult(const char* str, const char* uuid, char(&handle)[5]);
};

#endif

