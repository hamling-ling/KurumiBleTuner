#ifndef _PEAKDETECTMACHINEFP_H_
#define _PEAKDETECTMACHINEFP_H_

#include "PeakDetectMachineCommon.h"
#include "OsakanaFp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	typedef struct _PeakInfoFp_t {
		Fp_t value;
		uint16_t index;
	} PeakInfoFp_t;

	//struct _MachineContextFp_t;
	typedef struct _MachineContextFp_t MachineContextFp_t;

	MachineContextFp_t* CreatePeakDetectMachineContextFp();
	void DestroyPeakDetectMachineContextFp(MachineContextFp_t* ctx);
	void InputFp(MachineContextFp_t* ctx, Fp_t x);
	void ResetMachineFp(MachineContextFp_t* ctx);
	void GetKeyMaximumsFp(MachineContextFp_t* ctx, Fp_t filter, PeakInfoFp_t* list, int listmaxlen, int *num);
	bool ParabolicInterpFp(MachineContextFp_t* ctx, int index, Fp_t* xs, int sampleNum, Fp_t* x);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
