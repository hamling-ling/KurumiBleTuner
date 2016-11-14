#ifndef _MELODYCOMMANDRECEIVER_H_
#define _MELODYCOMMANDRECEIVER_H_

#include <stdint.h>

typedef uint8_t MelodyCommandEvent_t;

static const uint8_t kMelodyCommandEvtNone = 0;
static const uint8_t kMelodyCommandEvtExcited = 1;
static const uint8_t kMelodyCommandEvtFired = 2;

typedef struct MelodyCommand_tag {
	const uint16_t* melody0;
	const uint16_t* melody1;
	uint8_t melody0_len;
	uint8_t melody1_len;
} MelodyCommand_t;

typedef struct MelodyCommandResponse_tag {

	uint8_t commandIdx;
	MelodyCommandEvent_t evt;

	bool IsEmpty() {
		return (commandIdx == 0 && evt == kMelodyCommandEvtNone);
	}
} MelodyCommandResponse_t;

class ResponsiveMelodyDetector;

class MelodyCommandReceiver
{
public:
	MelodyCommandReceiver();
	MelodyCommandReceiver(const MelodyCommand_t* commands, int length);
	~MelodyCommandReceiver();

	bool Initialize(const MelodyCommand_t* commands, int length);
	MelodyCommandResponse_t Input(uint16_t value);

private:
	int _commandNum;
	ResponsiveMelodyDetector** _commands;
	uint8_t _excitedIdx;
	typedef MelodyCommandResponse_t(MelodyCommandReceiver::*InputFunc_t)(uint16_t);
	InputFunc_t _inputFunc;

	MelodyCommandResponse_t BaseStateInput(uint16_t value);
	MelodyCommandResponse_t ExcitedStateInput(uint16_t value);

	void ResetAllDetectors();
};

#endif
