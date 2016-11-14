#ifndef _CONTINUITYDETECTOR_H_
#define _CONTINUITYDETECTOR_H_

#include <stdint.h>

#define HISTROY_LEN	3

class ContinuityDetector
{
public:
	ContinuityDetector();
	~ContinuityDetector();
	/**
	@returns true if three identical values continued
	*/
	bool Input(uint16_t val);
	void Reset();
private:
	uint16_t _history[HISTROY_LEN];
	uint16_t _historyIndex;
};

#endif
