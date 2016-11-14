//#include "../include/EdgeDetector.h"
#include "EdgeDetector.h"
#include "ContinuityDetector.h"
#include "VolumeComparator.h"
#include <string.h>

EdgeDetector::EdgeDetector()
	:
	_lastNotifiedVal(0),
	_cd(new ContinuityDetector()),
	_vc(new VolumeComparator(256, 128))
{
}

EdgeDetector::~EdgeDetector()
{
	delete _cd;
	delete _vc;
}

bool EdgeDetector::Input(uint16_t value,  uint16_t volume)
{
	if (!_vc->Input(volume)) {
		// cutoff to small value
		value = 0;
	}

	if (_cd->Input(value)) {
		// 3 times continued value!
		if (_lastNotifiedVal != value) {
			_lastNotifiedVal = value;
			return true;
		}
	}
	return false;
}

uint16_t EdgeDetector::CurrentNote()
{
	return _lastNotifiedVal;
}

void EdgeDetector::Reset()
{
	_vc->Input(0);
	_cd->Reset();
	_lastNotifiedVal = 0;;
}
