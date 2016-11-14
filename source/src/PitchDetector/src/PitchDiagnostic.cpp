#include "PitchDiagnostic.h"

PitchDiagnostic::PitchDiagnostic(uint16_t interval)
	: kInterval(interval)
{
	Reset();
}


PitchDiagnostic::~PitchDiagnostic()
{
}

DiagnoseResult_t PitchDiagnostic::Diagnose(int8_t pitch, uint8_t note)
{
	if (pitch == INT8_MIN) {
		Reset();
		return kDiagnoseResultNone;
	}

	return (this->*_func)(pitch, note);
}

void PitchDiagnostic::Reset()
{
	_interval = 0;
	_sum = 0;
	_func = &PitchDiagnostic::diagnoseNoteOffState;
	_note = 0;
}

DiagnoseResult_t PitchDiagnostic::diagnoseNoteOnState(int8_t pitch, uint8_t note)
{
	if (note == 0) {
		Reset();
		return kDiagnoseResultNone;
	}

	if (note != _note) {
		Reset();
		return kDiagnoseResultNone;
	}

	if (pitch == INT8_MIN) {
		Reset();
		return kDiagnoseResultNone;
	}

	_sum += pitch;
	_interval++;
	if (kInterval <= _interval) {
		int kIntervalHalf = (int)(kInterval >> 1);
		DiagnoseResult_t ret = kDiagnoseResultGood;
		if (kIntervalHalf < _sum) {
			ret = kDiagnoseResultHigh;
		}
		else if (_sum < -kIntervalHalf) {
			ret = kDiagnoseResultLow;
		}
		// reset only interval. everthing else stay the same
		_interval = 0;
		return ret;
	}

	return kDiagnoseResultNone;
}

DiagnoseResult_t PitchDiagnostic::diagnoseNoteOffState(int8_t pitch, uint8_t note)
{
	if (note != 0) {
		if (note == INT8_MIN) {
			return kDiagnoseResultNone;
		}

		_sum = pitch;
		_func = &PitchDiagnostic::diagnoseNoteOnState;
		_interval++;
		_note = note;
	}

	return kDiagnoseResultNone;
}

