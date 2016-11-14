
#include <algorithm>
#include "../include/OsakanaPitchDetection.h"
#include "PeakDetectMachine.h"

osk_complex_t xf[N] = { { 0, 0 } };
float xf2[N2] = { 0 };
float _mf[N2] = { 0 };


PitchDetector::PitchDetector()
	: _fft(NULL), _det(NULL), _func(NULL)
{
}

PitchDetector::~PitchDetector()
{
	Cleanup();
}

int PitchDetector::Initialize(void* readFunc)
{
	_det = CreatePeakDetectMachineContext();

	if (InitOsakanaFft(&_fft, N, LOG2N) != 0) {
		DLOG("InitOsakanaFpFft error");
		return 1;
	}
	_func = (ReadDataFunc_t)readFunc;

	return 0;
}

void PitchDetector::Cleanup()
{
	CleanOsakanaFft(_fft);
	DestroyPeakDetectMachineContext(_det);
	_fft = NULL;
	_det = NULL;
}

int PitchDetector::DetectPitch(PitchInfo_t* pitchInfo)
{
	ResetMachine(_det);

	// sampling from analog pin
	DLOG("sampling...");
	_func(&xf[0].re, 2, N_ADC);
	DLOG("sampled");

	DLOG("raw data --");
	DRAWDATAf(xf, DEBUG_OUTPUT_NUM);

	DLOG("normalizing...");
	for (int i = 0; i < N2; i++) {
		xf[i].re -= 512.0f;
		xf[i].re /= 512.0f;
		xf[i].im = 0.0f;
		xf[N2 + i].re = 0.0f;
		xf[N2 + i].im = 0.0f;
		xf2[i] = xf[i].re * xf[i].re;
	}
	DLOG("normalized");

	DLOG("-- normalized input signal");
	DCOMPLEX(xf, DEBUG_OUTPUT_NUM);

	DLOG("-- fft/N");
	OsakanaFft(_fft, xf);
	DCOMPLEX(xf, DEBUG_OUTPUT_NUM);

	DLOG("-- power spectrum");
	for (int i = 0; i < N; i++) {
		xf[i].re = xf[i].re * xf[i].re + xf[i].im * xf[i].im;
		xf[i].im = 0.0f;
	}
	DCOMPLEX(xf, DEBUG_OUTPUT_NUM);

	DLOG("-- IFFT");
	OsakanaIfft(_fft, xf);
	DCOMPLEX(xf, DEBUG_OUTPUT_NUM);

	_mf[0] = xf[0].re * 2.0f;// why 2?
	for (int t = 1; t < N2; t++) {
		_mf[t] = _mf[t - 1] - xf2[t - 1];
	}

	DLOG("-- ms smart");
	DFPS(_mf, DEBUG_OUTPUT_NUM);

	// nsdf
	float* _nsdf = _mf; // reuse buffer
	for (int t = 0; t < N2; t++) {
		float mt = _mf[t]; // add small number to avoid 0 div
		_nsdf[t] = xf[t].re / mt;
		_nsdf[t] = _nsdf[t] * 2.0f;
	}
	DLOG("-- _nsdf");
	DFPS(_nsdf, DEBUG_OUTPUT_NUM);

	DLOG("-- pitch detection");
	for (int i = 0; i < N2; i++) {
		Input(_det, _nsdf[i]);
	}

	PeakInfo_t keyMaximums[4] = { 0 };
	int keyMaxLen = 0;
	GetKeyMaximums(_det, 0.5f, keyMaximums, sizeof(keyMaximums) / sizeof(PeakInfo_t), &keyMaxLen);
	if (0 < keyMaxLen) {
		float delta = 0;
		if (ParabolicInterp(_det, keyMaximums[0].index, _nsdf, N2, &delta)) {
			DLOG("delta %f\n", delta);
		}

		float freq = FREQ_PER_SAMPLE / (keyMaximums[0].index + delta);
		const float k = log10f(pow(2.0f, 1.0f / 12.0f));
		uint16_t midi = (uint16_t)round(log10f(freq / 27.5f) / k) + 21;
		DLOG("freq=%f Hz, note=%s\n", freq, kNoteStrings[midi % 12]);

		pitchInfo->freq = (uint16_t)round(freq);
		pitchInfo->midiNote = (uint8_t)midi;
		pitchInfo->noteStr = kNoteStrings[midi % 12];
	}

	DLOG("finished");
	return 0;
}

