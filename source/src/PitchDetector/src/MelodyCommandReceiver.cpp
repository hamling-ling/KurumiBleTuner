#include "MelodyCommandReceiver.h"
#include "ResponsiveMelodyDetector.h"
#include <cstdlib>
#include <cstring>

static const MelodyCommandResponse_t kEmptyResp = { 0,0 };

MelodyCommandReceiver::MelodyCommandReceiver(const MelodyCommand_t* commands, int length)
	:
	_commandNum(length),
	_commands(NULL)
{
	Initialize(commands, length);
}

MelodyCommandReceiver::MelodyCommandReceiver()
	:
	_commandNum(0)
{
	_commands = NULL;
}

MelodyCommandReceiver::~MelodyCommandReceiver()
{
	for (int i = 0; i < _commandNum; i++) {
		delete _commands[i];
	}
	free(_commands);
	_commands = NULL;
}

bool MelodyCommandReceiver::Initialize(const MelodyCommand_t* commands, int length)
{
	if (_commands != NULL) {
		return false;
	}

	_commandNum = length;
	_commands = static_cast<ResponsiveMelodyDetector**>(malloc(sizeof(ResponsiveMelodyDetector*) * length));
	for (int i = 0; i < _commandNum; i++) {
		const MelodyCommand_t* cmd = &(commands[i]);
		_commands[i] = new ResponsiveMelodyDetector(cmd->melody0, cmd->melody0_len, cmd->melody1, cmd->melody1_len);
	}
	ResetAllDetectors();

	return true;
}

MelodyCommandResponse_t MelodyCommandReceiver::Input(uint16_t value)
{
	return (this->*_inputFunc)(value);
}

MelodyCommandResponse_t MelodyCommandReceiver::ExcitedStateInput(uint16_t value)
{
	ResponsiveMelodyDetector* det = _commands[_excitedIdx];
	int result = det->Input(value);

	MelodyCommandResponse_t resp = kEmptyResp;
	switch (result) {
	case 0:
		break;
	case -1:
		ResetAllDetectors();
		break;
	case 2:
		resp.evt = kMelodyCommandEvtFired;
		resp.commandIdx = _excitedIdx;
		ResetAllDetectors();
		break;
	default:
		break;
	}

	return resp;
}

MelodyCommandResponse_t MelodyCommandReceiver::BaseStateInput(uint16_t value)
{
	MelodyCommandResponse_t resp = kEmptyResp;

	for (int i = 0; i < _commandNum; i++) {
		ResponsiveMelodyDetector* det = _commands[i];
		if (det->Input(value) == 1) {
			resp.commandIdx = i;
			resp.evt = kMelodyCommandEvtExcited;
			_inputFunc = &MelodyCommandReceiver::ExcitedStateInput;
			_excitedIdx = i;
			return resp;
		}
	}

	return resp;
}

void MelodyCommandReceiver::ResetAllDetectors()
{
	for (int i = 0; i < _commandNum; i++) {
		ResponsiveMelodyDetector* det = _commands[i];
		det->Reset();
	}
	_excitedIdx = 0;
	_inputFunc = &MelodyCommandReceiver::BaseStateInput;
}
