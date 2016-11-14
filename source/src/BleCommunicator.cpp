#include "BleCommunicator.h"
#include "Arduino.h"

#define LOG     Serial
#define RN4020  Serial1

#define DEBUG(...) { char debug_buf[256] = {0}; snprintf(debug_buf, sizeof(debug_buf), __VA_ARGS__); LOG.println(debug_buf); }

#define UUID_TUNER_SERVICE		"123456789012345678901234567890FF"
#define UUID_FREQ_CHAR			"12345678901234567890123456789033"
#define UUID_GAIN_CHAR			"12345678901234567890123456789034"

#define CMD_FACTORY_RESET		"SF,1"
#define CMD_FUNCTIONS			"SR,24000000"
#define CMD_SERVICE				"SS,C0000001"
#define CMD_DEVICE_NAME			"SN,KurumiTuner"
#define CMD_SER_DEVICE_NAME		"S-,KurumiTuner"
#define CMD_RESETSERVICES		"PZ"
#define CMD_PRIVATE_SERVICE		"PS," UUID_TUNER_SERVICE
#define CMD_PRIVATE_CHAR0		"PC," UUID_FREQ_CHAR ",12,04"
#define CMD_PRIVATE_CHAR1		"PC," UUID_GAIN_CHAR ",06,04"
#define CMD_VER_FW				"SDF,0.1"
#define CMD_VER_DEV				"SDH,0.1"
#define CMD_VER_SW				"SDR,0.1"
#define CMD_MODEL_NAME			"SDM,KurumiTuner"
#define CMD_MANUFACTURER		"SDN,osakanazabuun"
#define CMD_REBOOT				"R,1"

#define _countof(x) (sizeof(x) / sizeof (x[0]))
#define _strlen(s) (sizeof(s)/sizeof(s[0]))

typedef struct _SerialCommand {
	const char* cmd;
	const int cmd_len;
	const char* resp_ok;
	const char* resp_err;
	const int timeout;
} SerialCommand_t;

static const int kCmdRetry = 3;
static const char* kOk = "AOK";
static const char* kErr = "ERR";
static const char* kReboot = "Reboot";

static const SerialCommand_t kInitCommands[] = {
	{CMD_FACTORY_RESET,		_strlen(CMD_FACTORY_RESET),		kOk, kErr, 100},
	{CMD_FUNCTIONS,			_strlen(CMD_FUNCTIONS),			kOk, kErr, 100},
	{CMD_SERVICE,			_strlen(CMD_SERVICE),			kOk, kErr, 100},
	{CMD_DEVICE_NAME,		_strlen(CMD_DEVICE_NAME),		kOk, kErr, 100},
	{CMD_SER_DEVICE_NAME,	_strlen(CMD_SER_DEVICE_NAME),	kOk, kErr, 100},
	{CMD_RESETSERVICES,		_strlen(CMD_RESETSERVICES),		kOk, kErr, 100},
	{CMD_PRIVATE_SERVICE,	_strlen(CMD_PRIVATE_SERVICE),	kOk, kErr, 100},
	{CMD_PRIVATE_CHAR0,		_strlen(CMD_PRIVATE_CHAR0),		kOk, kErr, 100},
	{CMD_PRIVATE_CHAR1,		_strlen(CMD_PRIVATE_CHAR1),		kOk, kErr, 100},
	{CMD_VER_FW,			_strlen(CMD_VER_FW),			kOk, kErr, 100},
	{CMD_VER_DEV,			_strlen(CMD_VER_DEV),			kOk, kErr, 100},
	{CMD_VER_SW,			_strlen(CMD_VER_SW),			kOk, kErr, 100},
	{CMD_MODEL_NAME,		_strlen(CMD_MODEL_NAME),		kOk, kErr, 100},
	{CMD_MANUFACTURER,		_strlen(CMD_MANUFACTURER),		kOk, kErr, 100},
	{CMD_REBOOT,			_strlen(CMD_REBOOT),			kReboot, kErr, 1000},
};

BleCommunicator::BleCommunicator()
{
}

/**
 *  never tested
 */
BleCommunicator::~BleCommunicator()
{
}

uint32_t BleCommunicator::Initialize()
{
    DEBUG("%s ENT", __FUNCTION__);

    RN4020.begin(115200);
    
    int counter = 0;
    while(counter < 3) {
        if(initializeDevice()) {
            break;
        }
    }
	
    DEBUG("%s EXT", __FUNCTION__);
    return 0;
}

bool BleCommunicator::initializeDevice()
{
	for(int i = 0; i < _countof(kInitCommands); i++) {
		purge();
		
		const SerialCommand_t* pCmd = &kInitCommands[i];
		RN4020.setTimeout(pCmd->timeout);
		
		const char* cmd = pCmd->cmd;
		LOG.println(cmd);
		RN4020.println(cmd);
		
		String recv = RN4020.readStringUntil('\n');
		LOG.println("received:");
		LOG.println(recv);
		if(recv.startsWith(pCmd->resp_ok)) {
			LOG.println("success");
		} else {
			LOG.println("fail");
			return false;
		}
	}
	return true;
}

void BleCommunicator::purge()
{
    RN4020.begin(115200);
    while(RN4020.available() > 0) {
        char c = RN4020.read();
        LOG.print(c);
        delay(10);
    }
}

void BleCommunicator::UpdateFreq(uint16_t freq, uint16_t note)
{
    DEBUG("%s ENT", __FUNCTION__);
    if(_pitchVal[0] == freq && _pitchVal[1] == note) {
        DEBUG("%s EXT", __FUNCTION__);
        return;
    }
    DEBUG("purging");
    recvCommand();
    DEBUG("purged");
    
    uint16_t pitch[] = {freq, note};
    DEBUG("%s freq=%d, note=%d sent", __FUNCTION__, freq, note);
    
    _pitchVal[0] = pitch[0];
    _pitchVal[1] = pitch[2];
    RN4020.print("SUW," UUID_FREQ_CHAR ",");
    
    char buf[9] = {'\0'};
    snprintf(buf, sizeof(buf), "%04x%04x", freq, note);
    DEBUG(buf);
    RN4020.println(buf);
    
    // read response
    recvCommand();
		
    DEBUG("%s EXT", __FUNCTION__);
}

uint8_t BleCommunicator::GetGain()
{
    return _gainVal;
}

void BleCommunicator::recvCommand()
{
    char buf[32] = {0};
    while(RN4020.available() > 0) {
        String recv = RN4020.readStringUntil('\n');
	    LOG.println("received:");
	    LOG.println(recv);
	    
	    recv.toCharArray(buf, sizeof(buf));
	    if(strncmp(buf, "WV,001B,0", 9) == 0) {
	        if(buf[9] == '1') {
	            _gainVal = 1;
	            DEBUG("gain set to 1");
	        } else {
	            _gainVal = 0;
	            DEBUG("gain set to 0");
	        }
	    }
    }
}
    
uint32_t BleCommunicator::Yield()
{
    recvCommand();
    return 0;
}
