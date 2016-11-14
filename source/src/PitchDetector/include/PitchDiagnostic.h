#ifndef _PITCHDIAGNOSTIC_H_
#define _PITCHDIAGNOSTIC_H_

#include <stdint.h>

typedef uint8_t DiagnoseResult_t;

#define kDiagnoseResultNone		0
#define kDiagnoseResultHigh		1
#define kDiagnoseResultLow		2
#define kDiagnoseResultGood		3

class PitchDiagnostic
{
public:
	PitchDiagnostic(uint16_t interval);
	~PitchDiagnostic();
	DiagnoseResult_t Diagnose(int8_t pitch, uint8_t note);
	void Reset();

private:
	const uint16_t kInterval;
	uint16_t _interval;
	int16_t _sum;
	uint8_t _note;
	typedef DiagnoseResult_t(PitchDiagnostic::*func_t)(int8_t, uint8_t);
	func_t _func;

	DiagnoseResult_t diagnoseNoteOnState(int8_t pitch, uint8_t note);
	DiagnoseResult_t diagnoseNoteOffState(int8_t pitch, uint8_t note);
};

#endif
