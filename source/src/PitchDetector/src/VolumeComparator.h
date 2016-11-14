#ifndef _VOLUMECOMPARATOR_H_
#define _VOLUMECOMPARATOR_H_

#include <stdint.h>

#define HISTROY_LEN	3

typedef enum VolumeComparatorState_ {
	VolumeComparatorStateActive,
	VolumeComparatorStateInactive
} VolumeComparatorState_t;

class VolumeComparator
{
public:
	VolumeComparator(uint16_t off2OnThreshold, uint16_t on2offThreshold);
	~VolumeComparator();
	/**
	 *	@return true if ON, false otherwise
	 */
	bool Input(uint16_t val);
private:
	const uint16_t _off2OnThreshold;
	const uint16_t _on2offThreshold;

	VolumeComparatorState_t _state;
	typedef bool (VolumeComparator::*func_t)(uint16_t);
	func_t _func;

	bool InputActiveState(uint16_t val);
	bool InputInactiveState(uint16_t val);
};

#endif
