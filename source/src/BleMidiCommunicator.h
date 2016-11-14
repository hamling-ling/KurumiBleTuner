#ifndef _BLEMIDICOMMUNICATOR_H_
#define _BLEMIDICOMMUNICATOR_H_

#include "Communicator.h"

class Rn4020Controller;

#define NOTE_NONE    0
#define NOTE_C0		60
#define NOTE_C0s	61
#define NOTE_D0b	61
#define NOTE_D0		62
#define NOTE_D0s	63
#define NOTE_E0b	63
#define NOTE_E0		64
#define NOTE_F0		65
#define NOTE_F0s	66
#define NOTE_G0b	66
#define NOTE_G0		67
#define NOTE_G0s	68
#define NOTE_A0b	68
#define NOTE_A0		69
#define NOTE_A0s	70
#define NOTE_B0b	70
#define NOTE_B0		71
#define NOTE_C1		72
#define NOTE_C1s	73
#define NOTE_D1b	73
#define NOTE_D1		74
#define NOTE_D1s	75
#define NOTE_E1b	75
#define NOTE_E1		76
#define NOTE_F1		77
#define NOTE_F1s	78
#define NOTE_G1b	78
#define NOTE_G1		79
#define NOTE_G1s	80
#define NOTE_A1b	80
#define NOTE_A1		81
#define NOTE_A1s	82
#define NOTE_B1b	82
#define NOTE_B1		83
#define NOTE_C2		84

#define kInstTrp    2   // Trumpet
#define kInstPno    3   // Piano
#define kInstOrc    4   // Orchestra
#define kInstHmk    5   // HATSUNE Miku

#define kTuneNone   0
#define kTuneGood   1
#define kTuneHigh   2
#define kTuneLow    3

// should move these data to BleMidiCommunicator
// fuga G D Bb A G Bb A G F# A D
extern const uint16_t kMelFuga0[];
extern const uint16_t kMelFuga1[];
extern const uint8_t kMelFuga0Count;
extern const uint8_t kMelFuga1Count;

//  pic G F Bb C F D C F D Bb C G F
extern const uint16_t kMelPic0[];
extern const uint16_t kMelPic1[];
extern const uint8_t kMelPic0Count;
extern const uint8_t kMelPic1Count;

// bethooben G G G Eb F F F D
extern const uint16_t kMelBet0[];
extern const uint16_t kMelBet1[];
extern const uint8_t kMelBet0Count;
extern const uint8_t kMelBet1Count;

// eli E Eb E Eb E B D C A
extern const uint16_t kMelEli0[];
extern const uint16_t kMelEli1[];
extern const uint8_t kMelEli0Count;
extern const uint8_t kMelEli1Count;

// dov Eb F Gb F Eb Eb Eb C# Bb C# Eb
extern const uint16_t kMelDev0[];
extern const uint16_t kMelDev1[];
extern const uint8_t kMelDev0Count;
extern const uint8_t kMelDev1Count;

// blk F Bb A F D Eb D C Bb Bb F F Bb C Bb C
extern const uint16_t kMelBlk0[];
extern const uint16_t kMelBlk1[];
extern const uint8_t kMelBlk0Count;
extern const uint8_t kMelBlk1Count;

class BleMidiCommunicator : public Communicator
{
public:
    BleMidiCommunicator();
    ~BleMidiCommunicator();
    
    virtual uint32_t Initialize();
    virtual uint32_t Yield();
    virtual void UpdateFreq(uint16_t freq, uint16_t note);
    virtual uint8_t GetGain();
    virtual bool IsTuningMode();
    virtual void ConfirmaToggleGain();
    virtual void ToggleGain();
    virtual void ConfirmInstChange(uint8_t inst);
    virtual void ChangeInst(uint8_t inst);
    virtual void ConfirmToggleTuning();
    virtual void ToggleTuning();
    virtual void NotifyTune(uint8_t tune);
    
private:
    char m_noteCmdBuf[32];
    int8_t m_selected[16];        // user selected channels and note to play
    uint8_t m_channels[16]; // curretn midi note of channel
    Rn4020Controller* m_ble;
    bool m_isTuningMode;
    
    bool initializeDevice();
    void userNoteOff(uint16_t note);
    void userNoteOn(uint16_t note);
    void noteOff(uint8_t ch, uint16_t note);
    void noteOn(uint8_t ch, uint16_t note);
    void playMelody(uint8_t id);
    void programChannel(uint8_t ch, uint8_t inst);
    void setLylic(uint8_t lylic);
};

#endif //_BLEMIDICOMMUNICATOR_H_
