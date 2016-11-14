#include "ResponsiveMelodyDetector.h"
#include "MelodyDetector.h"

#if defined(ARDUINO_PLATFORM) || defined(RLDUINO78_VERSION) || defined(ARDUINO)      // arduino
#include <Arduino.h>
#else
#include <cstdlib>
#include <algorithm>
#endif

using namespace std;

ResponsiveMelodyDetector::ResponsiveMelodyDetector(const uint16_t* mel0, int mel0_len, const uint16_t* mel1, int mel1_len)
	:
	_pos(0)
{
	_mds[0] = new MelodyDetector(mel0, mel0_len);
	_mds[1] = new MelodyDetector(mel1, mel1_len);
}

ResponsiveMelodyDetector::~ResponsiveMelodyDetector()
{
	delete _mds[0];
	_mds[0] = NULL;
	delete _mds[1];
	_mds[1] = NULL;
}

int ResponsiveMelodyDetector::Input(uint16_t value)
{
	if (value == 0) {
		return false;
	}

	MelodyDetector* md = _mds[_pos];

	int result = md->Input(value);
	if (-1 == result) {
		_pos = 0;
		return -1;
	}

	if (0 == result) {
		return 0;
	}

	if (1 == result) {
		_pos++;
		if (_pos == MAX_MELODY_DETECTORS) {
			_pos = 0;
			return MAX_MELODY_DETECTORS;
		}
		return _pos;
	}

	return 0;
}

void ResponsiveMelodyDetector::Reset()
{
	_pos = 0;
	for (int i = 0; i < MAX_MELODY_DETECTORS; i++) {
		_mds[i]->Reset();
	}
}
