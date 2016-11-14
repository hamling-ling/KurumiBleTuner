#ifndef _OSAKANAPITCHDETECTION_H_
#define _OSAKANAPITCHDETECTION_H_

#include "OsakanaPitchDetectionCommon.h"

typedef struct _OsakanaFftContext_t OsakanaFftContext_t;
typedef struct _MachineContext_t MachineContext_t;

typedef int(*ReadDataFunc_t)(float* data, uint8_t stride, const int dataNum);


class PitchDetector : BasePitchDetector
{
public:
	PitchDetector();
	virtual ~PitchDetector();
	virtual int Initialize(void* readFunc);
	virtual void Cleanup();
	virtual int DetectPitch(PitchInfo_t* pitchInfo);

private:
	OsakanaFftContext_t* _fft;
	MachineContext_t* _det;
	ReadDataFunc_t _func;
};


#endif
