#ifndef OSAKANAPITCHDETECTIONFP_H_
#define OSAKANAPITCHDETECTIONFP_H_

#include "OsakanaPitchDetectionCommon.h"

typedef struct _OsakanaFpFftContext_t OsakanaFpFftContext_t;
typedef struct _MachineContextFp_t MachineContextFp_t;

typedef int (*ReadFpDataFunc_t)(Fp_t* data, uint8_t stride, const int dataNum, Fp_t* rawdata_min, Fp_t* rawdata_max);

class PitchDetectorFp : BasePitchDetector
{
public:
	PitchDetectorFp();
	virtual ~PitchDetectorFp();
	virtual int Initialize(void* readFunc);
	virtual void Cleanup();
	virtual int DetectPitch(PitchInfo_t* pitchInfo);

private:
	OsakanaFpFftContext_t* _fft;
	MachineContextFp_t* _det;
	ReadFpDataFunc_t _func;

	int8_t GetAccuracy(uint16_t note, uint16_t idx8);
};

#endif
