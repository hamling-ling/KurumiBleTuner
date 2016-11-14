
#include "MelodyDetector.h"

#if defined(ARDUINO_PLATFORM) || defined(RLDUINO78_VERSION) || defined(ARDUINO)      // arduino
#include <Arduino.h>
#else
#include <cstdlib>
#include <algorithm>
#endif


using namespace std;

MelodyDetector::MelodyDetector(const uint16_t* melody, uint8_t melodyLen)
	:
	_melodyLength(melodyLen),
	_pos(0)
{
	memset(_melody, 0, sizeof(_melody));
	memcpy(_melody, melody, sizeof(uint16_t) * min(melodyLen, (uint8_t)MAX_MELODY_LENGTH));
}

MelodyDetector::~MelodyDetector()
{
}

int MelodyDetector::Input(uint16_t value)
{
	if (value == 0) {
		return 0;
	}

	if (_melody[_pos] != value) {
		_pos = 0;
		return -1;
	}

	_pos++;
	if (_pos == _melodyLength) {
		_pos = 0;
		return 1;
	}

	return 0;
}

void MelodyDetector::Reset()
{
	_pos = 0;
}
