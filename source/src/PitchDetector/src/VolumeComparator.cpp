#include "VolumeComparator.h"
#include <string.h>

VolumeComparator::VolumeComparator(uint16_t off2OnThreshold, uint16_t on2offThreshold)
	:
	_off2OnThreshold(off2OnThreshold),
	_on2offThreshold(on2offThreshold),
	_state(VolumeComparatorStateInactive),
	_func(&VolumeComparator::InputInactiveState)
{
}

VolumeComparator::~VolumeComparator()
{
}

bool VolumeComparator::Input(uint16_t val)
{
	return (this->*_func)(val);
}

bool VolumeComparator::InputActiveState(uint16_t val)
{
	if (val < _on2offThreshold) {
		_state = VolumeComparatorStateInactive;
		_func = &VolumeComparator::InputInactiveState;
		return false;
	}
	return true;
}

bool VolumeComparator::InputInactiveState(uint16_t val)
{
	if (_off2OnThreshold <= val) {
		_state = VolumeComparatorStateActive;
		_func = &VolumeComparator::InputActiveState;
		return true;
	}
	return false;
}
