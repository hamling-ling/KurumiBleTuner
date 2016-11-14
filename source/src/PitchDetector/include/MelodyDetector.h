#ifndef _MELODYDETECTOR_H_
#define _MELODYDETECTOR_H_

#include <stdint.h>

#define MAX_MELODY_LENGTH	8

class MelodyDetector
{
public:
	MelodyDetector(const uint16_t* melody, uint8_t melodyLen);
	~MelodyDetector();

	/**
	 *	@return -1:state backed to initial
	 *	@return 0:state not changed
	 *	@return 1:detected and state backed to initial
	 */
	int Input(uint16_t value);
	void Reset();

private:
	uint8_t _melodyLength;
	uint8_t _pos;
	uint16_t _melody[MAX_MELODY_LENGTH];
};

#endif
