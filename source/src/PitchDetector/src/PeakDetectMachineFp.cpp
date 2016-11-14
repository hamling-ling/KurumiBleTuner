#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "PeakDetectMachineFp.h"

static PeakDetectMachineEvent_t SearchingBell_DetectEvent(MachineContextFp_t* ctx, Fp_t x);
static PeakDetectMachineEvent_t WalkingOnBell_DetectEvent(MachineContextFp_t* ctx, Fp_t x);
static PeakDetectMachineEvent_t End_DetectEvent(MachineContextFp_t* ctx, Fp_t x);
static void ChangeState(MachineContextFp_t* ctx, PeakDetectMachineState_t state);

static void SeachingBell_PosCross(MachineContextFp_t* ctx, Fp_t x);
static void SeachingBell_NegCross(MachineContextFp_t* ctx, Fp_t x);
static void SeachingBell_NmlData(MachineContextFp_t* ctx, Fp_t x);
static void SeachingBell_EndOfData(MachineContextFp_t* ctx, Fp_t x);

static void WalkingOnBell_PosCross(MachineContextFp_t* ctx, Fp_t x);
static void WalkingOnBell_NegCross(MachineContextFp_t* ctx, Fp_t x);
static void WalkingOnBell_NmlData(MachineContextFp_t* ctx, Fp_t x);
static void WalkingOnBell_EndOfData(MachineContextFp_t* ctx, Fp_t x);

static void End_PosCross(MachineContextFp_t* ctx, Fp_t x);
static void End_NegCross(MachineContextFp_t* ctx, Fp_t x);
static void End_NmlData(MachineContextFp_t* ctx, Fp_t x);
static void End_EndOfData(MachineContextFp_t* ctx, Fp_t x);

typedef PeakDetectMachineEvent_t (*EventDetector_t)(MachineContextFp_t* ctx, Fp_t x);
typedef void(*StateFuncFp_t)(MachineContextFp_t* ctx, Fp_t x);

static StateFuncFp_t s_funcs[kStateNum][kEventNum] = {
	{ SeachingBell_PosCross,	SeachingBell_NegCross,	SeachingBell_NmlData,	SeachingBell_EndOfData },
	{ WalkingOnBell_PosCross,	WalkingOnBell_NegCross, WalkingOnBell_NmlData , WalkingOnBell_EndOfData },
	{ End_PosCross,				End_NegCross,			End_NmlData,			End_EndOfData }
};

static EventDetector_t s_eventDetectors[kStateNum] = {
	SearchingBell_DetectEvent, WalkingOnBell_DetectEvent, End_DetectEvent
};

typedef struct _MachineContextFp_t {
	uint16_t maxDataNum;
	uint16_t currentIndex;
	PeakInfoFp_t lastInput;
	// collection of key maximum for each bell
	PeakInfoFp_t keyMaxs[kKeyMax];
	uint16_t keyMaxsNum;
	// max of all bell
	PeakInfoFp_t globalKeyMax;
	// max of current bell
	PeakInfoFp_t localKeyMax;
	PeakDetectMachineState_t state;
	StateFuncFp_t(*funcs)[kEventNum];
	EventDetector_t* detectors;
} MachineContextFp_t;

/////////////////////////////////////////////////////////////////////
// Public
/////////////////////////////////////////////////////////////////////

MachineContextFp_t* CreatePeakDetectMachineContextFp()
{
	MachineContextFp_t* ctx = (MachineContextFp_t*)malloc(sizeof(MachineContextFp_t));
	if (ctx == NULL) {
		return NULL;
	}
	ResetMachineFp(ctx);

	return ctx;
}

void DestroyPeakDetectMachineContextFp(MachineContextFp_t* ctx)
{
	free(ctx);
}

void InputFp(MachineContextFp_t* ctx, Fp_t x)
{
	EventDetector_t detector = ctx->detectors[ctx->state];
	PeakDetectMachineEvent_t evt = detector(ctx, x);
	
	StateFuncFp_t stateFunc = ctx->funcs[ctx->state][(int)evt];
	stateFunc(ctx, x);
}

void ResetMachineFp(MachineContextFp_t* ctx)
{
	memset(ctx, 0, sizeof(MachineContextFp_t));
	ctx->funcs = s_funcs;
	ctx->detectors = s_eventDetectors;
}

void GetKeyMaximumsFp(MachineContextFp_t* ctx, Fp_t filter, PeakInfoFp_t* list, int listmaxlen, int *num)
{
	if (ctx->keyMaxsNum == 0) {
		*num = 0;
		return;
	}

	if (listmaxlen < 0) {
		*num = 0;
		return;
	}

	// threshold
	Fp_t th = filter;
	// elem num above threshold
	int counter = 1;

	if (ctx->globalKeyMax.value < th) {
		counter = 0;
		return;
	}

	// [0] is reserved for globalMax
	list[0] = ctx->globalKeyMax;
	for (int i = 0; i < ctx->keyMaxsNum && counter < listmaxlen; i++) {
		Fp_t keyMax = ctx->keyMaxs[i].value;
		if (filter * keyMax < th) {
			continue;
		}

		if (ctx->keyMaxs[i].index == list[0].index) {
			continue;
		}

		list[counter++] = ctx->keyMaxs[i];
	}
	*num = counter;
}

 bool ParabolicInterpFp(MachineContextFp_t* ctx, int index, Fp_t* xs, int sampleNum, Fp_t* x)
{
	if (0 == index || sampleNum <= index + 1) {
		return false;
	}
	
	//use Ragrange interpolation
	// consider (-1,y0),(0,y1),(1,y2)
	// at x = (y0-y2)/(2*(y0+y2)-4*y1), y'=0
	Fp_t y0 = xs[index - 1];
	Fp_t y1 = xs[index + 0];
	Fp_t y2 = xs[index + 1];

	Fp_t num = y0 - y2;
	Fp_t denom = ((y0 + y2) << 1) - (y1 << 2);
	if (denom == 0) {
		return false;
	}
	*x = FpDiv(num, denom);

	return true;
}

/////////////////////////////////////////////////////////////////////
// Private
/////////////////////////////////////////////////////////////////////

static void UpdateValueHistory(MachineContextFp_t* ctx, Fp_t x)
{
	ctx->lastInput.value = x;
	ctx->lastInput.index = ctx->currentIndex;
	ctx->currentIndex++;
}

static void UpdateLocalKeyMax(MachineContextFp_t* ctx, Fp_t x, uint16_t index)
{
	PeakInfoFp_t localMax = { x, index };
	ctx->localKeyMax = localMax;
}

static void PushLocalKeyMax(MachineContextFp_t* ctx)
{
	if (kKeyMax <= ctx->keyMaxsNum + 1) {
		return;
	}
	ctx->keyMaxs[ctx->keyMaxsNum] = ctx->localKeyMax;
	ctx->keyMaxsNum++;
	if (ctx->globalKeyMax.value < ctx->localKeyMax.value) {
		ctx->globalKeyMax = ctx->localKeyMax;
	}
}

static PeakDetectMachineEvent_t SearchingBell_DetectEvent(MachineContextFp_t* ctx, Fp_t x) {
	if (ctx->lastInput.value < 0 && 0 <= x) {
		return kEventPosCross;
	}
	else if(ctx->maxDataNum <= ctx->currentIndex+1){
		return kEventNmlData;
	}
	else {
		return kEventEndOfData;
	}
}

static PeakDetectMachineEvent_t WalkingOnBell_DetectEvent(MachineContextFp_t* ctx, Fp_t x) {
	if (0 <= ctx->lastInput.value && x < 0) {
		return kEventNegCross;
	}
	else {
		return kEventNmlData;
	}
}

static PeakDetectMachineEvent_t End_DetectEvent(MachineContextFp_t* ctx, Fp_t x) {
	return kEventEndOfData;
}

static void ChangeState(MachineContextFp_t* ctx, PeakDetectMachineState_t state)
{
	ctx->state = state;
}

static void SeachingBell_PosCross(MachineContextFp_t* ctx, Fp_t x)
{
	// reset local max
	UpdateLocalKeyMax(ctx, x, ctx->currentIndex);
	UpdateValueHistory(ctx, x);
	ChangeState(ctx, kStateWalkingOnBell);
}

static void SeachingBell_NegCross(MachineContextFp_t* ctx, Fp_t x)
{
	UpdateValueHistory(ctx, x);
	ChangeState(ctx, kStateSearchingBell);
}

static void SeachingBell_NmlData(MachineContextFp_t* ctx, Fp_t x)
{
	UpdateValueHistory(ctx, x);
}

static void SeachingBell_EndOfData(MachineContextFp_t* ctx, Fp_t x)
{
	UpdateValueHistory(ctx, x);
	ChangeState(ctx, kStateEnd);
}

static void WalkingOnBell_PosCross(MachineContextFp_t* ctx, Fp_t x)
{
	assert(true);
}

static void WalkingOnBell_NegCross(MachineContextFp_t* ctx, Fp_t x)
{
	PushLocalKeyMax(ctx);
	UpdateValueHistory(ctx, x);
	ChangeState(ctx, kStateSearchingBell);
}

static void WalkingOnBell_NmlData(MachineContextFp_t* ctx, Fp_t x)
{
	// update local key max
	if (ctx->localKeyMax.value < x) {
		UpdateLocalKeyMax(ctx, x, ctx->currentIndex);
	}

	UpdateValueHistory(ctx, x);
}

static void WalkingOnBell_EndOfData(MachineContextFp_t* ctx, Fp_t x)
{
	UpdateValueHistory(ctx, x);
}

static void End_PosCross(MachineContextFp_t* ctx, Fp_t x)
{
	UpdateValueHistory(ctx, x);
}

static void End_NegCross(MachineContextFp_t* ctx, Fp_t x)
{
	UpdateValueHistory(ctx, x);
}

static void End_NmlData(MachineContextFp_t* ctx, Fp_t x)
{
	UpdateValueHistory(ctx, x);
}

static void End_EndOfData(MachineContextFp_t* ctx, Fp_t x)
{
	UpdateValueHistory(ctx, x);
}

