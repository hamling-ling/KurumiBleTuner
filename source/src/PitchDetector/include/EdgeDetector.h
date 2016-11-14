#ifndef _EDGEDETECTOR_H_
#define _EDGEDETECTOR_H_

#include <stdint.h>

#define HISTROY_LEN	3

class ContinuityDetector;
class VolumeComparator;

class EdgeDetector
{
public:
	EdgeDetector();
	~EdgeDetector();
	/**
	 *	detect note change.
	 *	@return	true when edge(value change) detected
	 *	@note	given value culd different from current note.
	 *			so use CurrentNote() to get latest internal value.
	 */
	bool Input(uint16_t value, uint16_t volume);
	uint16_t CurrentNote();
	void Reset();

private:
	uint16_t _lastNotifiedVal;
	ContinuityDetector* _cd;
	VolumeComparator* _vc;
};

#endif
