#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "PeakDetectMachine.h"

static PeakDetectMachineEvent_t SearchingBell_DetectEvent(MachineContext_t* ctx, float x);
static PeakDetectMachineEvent_t WalkingOnBell_DetectEvent(MachineContext_t* ctx, float x);
static PeakDetectMachineEvent_t End_DetectEvent(MachineContext_t* ctx, float x);
static void ChangeState(MachineContext_t* ctx, PeakDetectMachineState_t state);

static void SeachingBell_PosCross(MachineContext_t* ctx, float x);
static void SeachingBell_NegCross(MachineContext_t* ctx, float x);
static void SeachingBell_NmlData(MachineContext_t* ctx, float x);
static void SeachingBell_EndOfData(MachineContext_t* ctx, float x);

static void WalkingOnBell_PosCross(MachineContext_t* ctx, float x);
static void WalkingOnBell_NegCross(MachineContext_t* ctx, float x);
static void WalkingOnBell_NmlData(MachineContext_t* ctx, float x);
static void WalkingOnBell_EndOfData(MachineContext_t* ctx, float x);

static void End_PosCross(MachineContext_t* ctx, float x);
static void End_NegCross(MachineContext_t* ctx, float x);
static void End_NmlData(MachineContext_t* ctx, float x);
static void End_EndOfData(MachineContext_t* ctx, float x);

typedef PeakDetectMachineEvent_t (*EventDetector_t)(MachineContext_t* ctx, float x);
typedef void(*StateFunc_t)(MachineContext_t* ctx, float x);

static StateFunc_t s_funcs[kStateNum][kEventNum] = {
	{ SeachingBell_PosCross,	SeachingBell_NegCross,	SeachingBell_NmlData,	SeachingBell_EndOfData },
	{ WalkingOnBell_PosCross,	WalkingOnBell_NegCross, WalkingOnBell_NmlData , WalkingOnBell_EndOfData },
	{ End_PosCross,				End_NegCross,			End_NmlData,			End_EndOfData }
};

static EventDetector_t s_eventDetectors[kStateNum] = {
	SearchingBell_DetectEvent, WalkingOnBell_DetectEvent, End_DetectEvent
};

typedef struct _MachineContext_t {
	uint16_t maxDataNum;
	uint16_t currentIndex;
	PeakInfo_t lastInput;
	// collection of key maximum for each bell
	PeakInfo_t keyMaxs[kKeyMax];
	uint16_t keyMaxsNum;
	// max of all bell
	PeakInfo_t globalKeyMax;
	// max of current bell
	PeakInfo_t localKeyMax;
	PeakDetectMachineState_t state;
	StateFunc_t (*funcs)[kEventNum];
	EventDetector_t* detectors;
} MachineContext_t;

/////////////////////////////////////////////////////////////////////
// Public
/////////////////////////////////////////////////////////////////////

MachineContext_t* CreatePeakDetectMachineContext()
{
	MachineContext_t* ctx = (MachineContext_t*)malloc(sizeof(MachineContext_t));
	if (ctx == NULL) {
		return NULL;
	}
	ResetMachine(ctx);

	return ctx;
}

void DestroyPeakDetectMachineContext(MachineContext_t* ctx)
{
	free(ctx);
}

void Input(MachineContext_t* ctx, float x)
{
	EventDetector_t detector = ctx->detectors[ctx->state];
	PeakDetectMachineEvent_t evt = detector(ctx, x);
	
	StateFunc_t stateFunc = ctx->funcs[ctx->state][(int)evt];
	stateFunc(ctx, x);
}

void ResetMachine(MachineContext_t* ctx)
{
	memset(ctx, 0, sizeof(MachineContext_t));
	ctx->funcs = s_funcs;
	ctx->detectors = s_eventDetectors;
}

void GetKeyMaximums(MachineContext_t* ctx, float filter, PeakInfo_t* list, int listmaxlen, int *num)
{
	if (ctx->keyMaxsNum == 0) {
		*num = 0;
		return;
	}

	if (listmaxlen < 0) {
		*num = 0;
		return;
	}

	// elem num above threshold
	float th = filter;
	// elem num above threshold
	int counter = 1;

	if (ctx->globalKeyMax.value < th) {
		counter = 0;
		return;
	}

	// [0] is reserved for globalMax
	list[0] = ctx->globalKeyMax;
	for (int i = 0; i < ctx->keyMaxsNum && counter < listmaxlen; i++) {
		float keyMax = ctx->keyMaxs[i].value;
		if (filter * keyMax < th) {
			continue;
		}

		if (i == list[0].index) {
			continue;
		}

		list[counter++] = ctx->keyMaxs[i];
	}
	*num = counter;
}

 bool ParabolicInterp(MachineContext_t* ctx, int index, float* xs, int sampleNum, float* x)
{
	if (0 == index || sampleNum <= index + 1) {
		return false;
	}
	
	//use Ragrange interpolation
	// consider (-1,y0),(0,y1),(1,y2)
	// at x = (y0-y2)/(2*(y0+y2)-4*y1), y'=0
	float y0 = xs[index - 1];
	float y1 = xs[index + 0];
	float y2 = xs[index + 1];
	
	float num = y0 - y2;
	float denom = 2.0f * (y0 + y2) - 4.0f * y1;
	if (denom == 0) {
		return false;
	}
	*x = num / denom;

	return true;
}

/////////////////////////////////////////////////////////////////////
// Private
/////////////////////////////////////////////////////////////////////

static void UpdateValueHistory(MachineContext_t* ctx, float x)
{
	ctx->lastInput.value = x;
	ctx->lastInput.index = ctx->currentIndex;
	ctx->currentIndex++;
}

static void UpdateLocalKeyMax(MachineContext_t* ctx, float x, uint16_t index)
{
	PeakInfo_t localMax = { x, index };
	ctx->localKeyMax = localMax;
}

static void PushLocalKeyMax(MachineContext_t* ctx)
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

static PeakDetectMachineEvent_t SearchingBell_DetectEvent(MachineContext_t* ctx, float x) {
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

static PeakDetectMachineEvent_t WalkingOnBell_DetectEvent(MachineContext_t* ctx, float x) {
	if (0 <= ctx->lastInput.value && x < 0) {
		return kEventNegCross;
	}
	else {
		return kEventNmlData;
	}
}

static PeakDetectMachineEvent_t End_DetectEvent(MachineContext_t* ctx, float x) {
	return kEventEndOfData;
}

static void ChangeState(MachineContext_t* ctx, PeakDetectMachineState_t state)
{
	ctx->state = state;
}

static void SeachingBell_PosCross(MachineContext_t* ctx, float x)
{
	// reset local max
	UpdateLocalKeyMax(ctx, x, ctx->currentIndex);
	UpdateValueHistory(ctx, x);
	ChangeState(ctx, kStateWalkingOnBell);
}

static void SeachingBell_NegCross(MachineContext_t* ctx, float x)
{
	UpdateValueHistory(ctx, x);
	ChangeState(ctx, kStateSearchingBell);
}

static void SeachingBell_NmlData(MachineContext_t* ctx, float x)
{
	UpdateValueHistory(ctx, x);
}

static void SeachingBell_EndOfData(MachineContext_t* ctx, float x)
{
	UpdateValueHistory(ctx, x);
	ChangeState(ctx, kStateEnd);
}

static void WalkingOnBell_PosCross(MachineContext_t* ctx, float x)
{
	assert(true);
}

static void WalkingOnBell_NegCross(MachineContext_t* ctx, float x)
{
	PushLocalKeyMax(ctx);
	UpdateValueHistory(ctx, x);
	ChangeState(ctx, kStateSearchingBell);
}

static void WalkingOnBell_NmlData(MachineContext_t* ctx, float x)
{
	// update local key max
	if (ctx->localKeyMax.value < x) {
		UpdateLocalKeyMax(ctx, x, ctx->currentIndex);
	}

	UpdateValueHistory(ctx, x);
}

static void WalkingOnBell_EndOfData(MachineContext_t* ctx, float x)
{
	UpdateValueHistory(ctx, x);
}

static void End_PosCross(MachineContext_t* ctx, float x)
{
	UpdateValueHistory(ctx, x);
}

static void End_NegCross(MachineContext_t* ctx, float x)
{
	UpdateValueHistory(ctx, x);
}

static void End_NmlData(MachineContext_t* ctx, float x)
{
	UpdateValueHistory(ctx, x);
}

static void End_EndOfData(MachineContext_t* ctx, float x)
{
	UpdateValueHistory(ctx, x);
}

