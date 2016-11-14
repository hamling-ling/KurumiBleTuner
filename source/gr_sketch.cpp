/*GR-KURUMI Sketch Template Version: V1.13*/
#include <Arduino.h>
#include "OsakanaFpFft.h"
#include "OsakanaPitchDetectionFp.h"
#include "EdgeDetector.h"
#include "StopWatch.h"
#include "BleCommunicator.h"
#include "BleMidiCommunicator.h"
#include "MelodyCommandReceiver.h"
#include "PitchDiagnostic.h"
#include "CommonTool.h"

#define USE_MIDI_OVER_BLE

//#define _DEBUG
#define LOG_PRINTF	Serial.print
#include "OsakanaFpFftDebug.h"

#define LOG_Serial			Serial
#define STOPWATCH(msg)      StopWatch sw(msg);
//#define STOPWATCH(msg)
//#define LOOPBREAK()         loopBreak()
#define LOOPBREAK()
//#define DEBUG_OUTPUT_NUM    256

#define ON	true
#define OFF	false

// Pin 22,23,24 are assigned to RGB LEDs.
int led_red   = 22; // LOW active
int led_green = 23; // LOW active
int led_blue  = 24; // LOW active
int led_pin   = 5;

#if defined (USE_MIDI_OVER_BLE)
BleMidiCommunicator com;
#else
BleCommunicator com;
#endif
static Communicator& s_com = com;
static PitchDetectorFp s_pitch;
static EdgeDetector s_edge;
static MelodyCommandReceiver s_mcr;
static PitchDiagnostic s_pd(20);

static inline void detectPitch(PitchInfo_t* pitchInfo);
static inline bool processMelodyCommand(uint16_t note);
static inline void processResult(PitchInfo_t* pitchInfo);
static inline void processPitchDiagnostic(int8_t pitch, bool edge);
static inline void readDataFp(Fp_t* data, uint8_t stride, const int dataNum, Fp_t* x_min, Fp_t* x_max);
static inline void SetLeds(bool red, bool green, bool blue, bool led);
static inline void GoToErrorState();
static inline void loopBreak();

void setup()
{
	LOG_Serial.begin(115200);

	analogReference(EXTERNAL);
	
	ILOG(LOG_NEWLINE "GR-KURUMI start");
	
	pinMode(led_red, OUTPUT);
	pinMode(led_green, OUTPUT);
	pinMode(led_blue, OUTPUT);
	pinMode(led_pin, OUTPUT);
	
	SetLeds(ON,ON,ON,ON);
	
	if(s_com.Initialize() != 0) {
		GoToErrorState();
	}
	digitalWrite(led_red, HIGH);
	if(s_pitch.Initialize((void*)readDataFp) != 0) {
		GoToErrorState();
	}
	digitalWrite(led_green, HIGH);
	
	{
		MelodyCommand_t commands[] = {
			{ kMelFuga0,	kMelFuga1,	kMelFuga0Count,	kMelFuga1Count },   //
			{ kMelBet0,		kMelBet1,	kMelBet0Count,	kMelBet1Count },    //
			{ kMelPic0,		kMelPic1,	kMelPic0Count,	kMelPic1Count },    // kInstTrp
			{ kMelEli0,		kMelEli1,	kMelEli0Count,	kMelEli1Count },    // kInstPno
			{ kMelDev0,		kMelDev1,	kMelDev0Count,	kMelDev1Count },    // kInstOrc
			{ kMelBlk0,		kMelBlk1,	kMelBlk0Count,	kMelBlk1Count },    // kInstHmk
		};
		
		if(!s_mcr.Initialize(commands, _countof(commands))) {
			GoToErrorState();
		}
	}
}

// the loop routine runs over and over again forever:
void loop() {
   /* initialize stuff */
    ILOG(LOG_NEWLINE "********* Starting Main Loop *********");
	SetLeds(OFF,OFF,OFF,OFF);
	
#if 1
    PitchInfo_t pitchInfo;
    
    ILOG("main loop");
    while(1) {
        LOOPBREAK();
		detectPitch(&pitchInfo);
        processResult(&pitchInfo);
        s_com.Yield();
    }
#else
    while(true) { delay(100);}
#endif
}

static void detectPitch(PitchInfo_t* pitchInfo)
{
	*pitchInfo = MakePitchInfo();
    
    digitalWrite(led_green, LOW);// active hight
    if(s_pitch.DetectPitch(pitchInfo) != 0) {
    	// could be invalid range signal
    }
    digitalWrite(led_green, HIGH);// active hight
    
    if(pitchInfo->midiNote != 0) {
  		digitalWrite(led_blue, LOW);// active hight
    } else {
        digitalWrite(led_blue, HIGH);// active hight
    }
}

static void processResult(PitchInfo_t* pitchInfo)
{
	//ILOG("note=%d, vol=%d", pitchInfo->midiNote, pitchInfo->volume);
	bool isEdge = s_edge.Input(pitchInfo->midiNote, pitchInfo->volume);
	uint16_t note = s_edge.CurrentNote();
	bool melodyProcessed = false;
    if(isEdge) {
    	if(note == 0) {
	        ILOG("edge! none");
            pitchInfo->midiNote = 0;
    	} else {
    		ILOG("edge! %s", pitchInfo->noteStr == NULL?"NULL":pitchInfo->noteStr);
    	}

        s_com.UpdateFreq(pitchInfo->freq, note);
        
        melodyProcessed = processMelodyCommand(note);
    } else {
#if !defined (USE_MIDI_OVER_BLE)
        if(note != 0) {
            if(note == pitchInfo->midiNote) {
            	// event note doesn't change, freq change shoud be notified
                s_com.UpdateFreq(pitchInfo->freq, note);
            }
        }
#endif
    }
    
    if(!melodyProcessed && s_com.IsTuningMode()) {
    	//ILOG("pitch=%d, note=%d", (int)pitchInfo->pitch, note);
    	processPitchDiagnostic(pitchInfo->pitch, note);
    }
}

static bool processMelodyCommand(uint16_t note)
{
	bool handled = false;
	MelodyCommandResponse_t resp = s_mcr.Input(note);
	if(resp.IsEmpty()) {
		return handled;
	}
	
	switch(resp.evt) {
		case kMelodyCommandEvtExcited:
    		ILOG("kMelodyCommandEvtExcited");
    		// wait sometime to finish current note
    		delay(1000);
    		// reset state
    		s_edge.Reset();
    		s_pd.Reset();
    		// play response melody id=resp.cmmandIdx
    		if(resp.commandIdx == 0) {
    			s_com.ConfirmaToggleGain();
    		} else if(resp.commandIdx == 1) {
    		    s_com.ConfirmToggleTuning();
    		} else {
    		   s_com.ConfirmInstChange(resp.commandIdx);
    		}
    		handled = true;
    		break;
		case kMelodyCommandEvtFired:
    		ILOG("kMelodyCommandEvtFired");
    		// wait sometime to finish current note
    		delay(1000);
    		// reset state
    		s_edge.Reset();
    		s_pd.Reset();
    		// execute command
    		if(resp.commandIdx == 0) {
    			// Change Gain
    			s_com.ToggleGain();
    		} else if(resp.commandIdx == 1) {
    			s_com.ToggleTuning();
    		    s_pd.Reset();
    		} else {
    			// Change Instrument
    			s_com.ChangeInst(resp.commandIdx);
    		}
    		handled = true;
			break;
		default:
			ILOG("unexpedted!");
			break;
	}
	return handled;
}

static void processPitchDiagnostic(int8_t pitch, bool edge)
{
	DiagnoseResult_t result = s_pd.Diagnose(pitch, edge);
	switch(result) {
		case kDiagnoseResultNone:
			break;
		case kDiagnoseResultGood:
		    ILOG("kDiagnoseResultGood");
            s_com.NotifyTune(kTuneGood);
			break;
		case kDiagnoseResultHigh:
		    ILOG("kDiagnoseResultHigh");
		    s_com.NotifyTune(kTuneHigh);
			break;
		case kDiagnoseResultLow:
	    	ILOG("kDiagnoseResultLow");
		    s_com.NotifyTune(kTuneLow);
			break;
		default:
			break;
	}
}

static void readDataFp(Fp_t* data, uint8_t stride, const int dataNum, Fp_t* x_min, Fp_t* x_max)
{
	int ain_pin = s_com.GetGain();
	
    *x_min = 512;
    *x_max = 0;
    int counter = 0;
    while (counter < dataNum) {
        *data = analogRead(ain_pin);

        *x_min = min(*data, *x_min);
        *x_max = max(*data, *x_max);
        
        data += stride;
        counter++;
    }
}

static void SetLeds(bool red, bool green, bool blue, bool led)
{
	digitalWrite(led_red, red?LOW:HIGH);
	digitalWrite(led_green, green?LOW:HIGH);
	digitalWrite(led_blue, blue?LOW:HIGH);
	digitalWrite(led_pin, led?HIGH:LOW);
}

static void GoToErrorState()
{
	digitalWrite(led_green, LOW);
	digitalWrite(led_blue, LOW);
	digitalWrite(led_pin, LOW);
	
	while(true) {
		digitalWrite(led_red, HIGH);
		delay(1000);
		digitalWrite(led_red, LOW);
		delay(1000);
	}
}

static void loopBreak()
{
	LOG_Serial.println("ready?");
	while(true) {
		if(LOG_Serial.available() > 0) {
			char input = LOG_Serial.read();
			if(input == 'g') {
				break;
			}
		}
		delay(100);
	}
}
