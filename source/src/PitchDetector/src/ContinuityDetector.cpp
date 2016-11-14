#include "ContinuityDetector.h"
#include <string.h>

ContinuityDetector::ContinuityDetector() : _historyIndex(0)
{
    memset(_history, 0, sizeof(_history));
}

ContinuityDetector::~ContinuityDetector()
{
}

bool ContinuityDetector::Input(uint16_t val)
{
	_history[_historyIndex] = val;
	_historyIndex = (_historyIndex + 1) % HISTROY_LEN;

	for (int i = 0; i < HISTROY_LEN; i++) {
		if (_history[i] != _history[0]) {
			return false;
		}
	}
	return true;
}

void ContinuityDetector::Reset()
{
	_historyIndex = 0;

	for (int i = 0; i < HISTROY_LEN; i++) {
		_history[i] = 0;
	}
}
