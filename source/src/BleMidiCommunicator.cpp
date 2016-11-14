#include "BleMidiCommunicator.h"
#include "Arduino.h"
#include <assert.h>
#include "Rn4020Controller.h"
#include "CommonTool.h"
#include "StringUtility.h"

#include "debug.h"

#define RN4020  Serial1

#define NOTE_BDRM   36
#define NOTE_SYM    49
#define NOTE_TRI    81

#define NOTELEN4    1000
#define NOTELEN16   (NOTELEN4>>2)
#define NOTELEN8    (NOTELEN4>>1)
#define NOTELEN2    (NOTELEN4<<1)
#define NOTELEN1    (NOTELEN4<<2)
#define NOTELEN3    (NOTELEN4/3)
#define NOTELEN6    (NOTELEN4/6)

// midi channel 0-15
#define CH_HMK      0   // Ch for HATSUNE Miku
#define CH_PNO      1   // Ch for Piano
#define CH_POR      2   // Ch for Organ
#define CH_TRP      3   // Ch for Trumpet
#define CH_GTR      5   // Ch for Guitar
#define CH_VLN      6   // Ch for Violin
#define CH_STR      7   // Ch for String ensemble
#define CH_BRA		8	// ch for Brass ensemble
#define CH_PCS      9   // Ch for Parcussion. Not user selectable
#define CH_TBA		10	// Ch for Tuba
#define CH_CTR		11	// Ch for Contrabass
#define CH_OBE		12	// Ch for Oboe
#define CH_PCC		13	// Ch for Piccolo

// midi instrument 0-127
#define INST_PNO     0  // Piano
#define INST_POR    19  // Organ
#define INST_TRP    56  // Trumpet
#define INST_PCS    35  // Parcussion
#define INST_GTR    30  // Guitar
#define INST_VLN    40  // Violin
#define INST_STR    48  // String Ensemble
#define INST_BRA	61	// Brass Ensemble
#define INST_TBA	58	// Tuba
#define INST_CTR	43	// Contrabass
#define	INST_OBE	68	// Oboe
#define	INST_PCC	72	// Piccolo

#define SYSEX_TEMPLATE	"8080F0437909110A002780F7"
//#define LYLIC			0x71	// ru
#define LYLIC			0x6F	// la

typedef struct Note_tag {
    uint16_t note;
    uint16_t delay;
} Note_t;

typedef struct Melody_tag {
    const uint8_t ch;
    const Note_t* playNotes;
    const uint8_t note_len;
} Melody_t;

static const int8_t kInvalidNote				= INT8_MIN;

// should move these data to BleMidiCommunicator
// fuga G D Bb A G Bb A G F# A D
const uint16_t kMelFuga0[] = { NOTE_G0, NOTE_D1, NOTE_B0b, NOTE_A0 };// G D Bb A 
const uint16_t kMelFuga1[] = { NOTE_A0, NOTE_D0 };// A D
const uint8_t kMelFuga0Count = _countof(kMelFuga0);
const uint8_t kMelFuga1Count = _countof(kMelFuga1);

//  pic G F Bb C F D C F D Bb C G F
const uint16_t kMelPic0[] = { NOTE_G0, NOTE_F0, NOTE_B0b, NOTE_C1 };// G F Bb C 
const uint16_t kMelPic1[] = { NOTE_G0, NOTE_F0 };// A D
const uint8_t kMelPic0Count = _countof(kMelPic0);
const uint8_t kMelPic1Count = _countof(kMelPic1);

// bethooben G G G Eb F F F D
const uint16_t kMelBet0[] = { NOTE_G0, NOTE_G0, NOTE_G0, NOTE_E0b };
const uint16_t kMelBet1[] = { NOTE_D0 };// A D
const uint8_t kMelBet0Count = _countof(kMelBet0);
const uint8_t kMelBet1Count = _countof(kMelBet1);

// eli E Eb E Eb E B D C A
const uint16_t kMelEli0[] = { NOTE_E1, NOTE_E1b, NOTE_E1, NOTE_E1b, NOTE_E1 };
const uint16_t kMelEli1[] = { NOTE_A0 };
const uint8_t kMelEli0Count = _countof(kMelEli0);
const uint8_t kMelEli1Count = _countof(kMelEli1);

// dov Eb F Gb F Eb Eb Eb C# Bb C# Eb
const uint16_t kMelDev0[] = { NOTE_E0b, NOTE_F0, NOTE_G0b, NOTE_F0, NOTE_E0b };
const uint16_t kMelDev1[] = { NOTE_C0s, NOTE_E0b };
const uint8_t kMelDev0Count = _countof(kMelDev0);
const uint8_t kMelDev1Count = _countof(kMelDev1);

// blk F Bb A F D Eb D C Bb Bb F F Bb C Bb C
const uint16_t kMelBlk0[] = { NOTE_F0, NOTE_B0b, NOTE_A0, NOTE_F0, NOTE_D1 };
const uint16_t kMelBlk1[] = { NOTE_B0b, NOTE_B0b };
const uint8_t kMelBlk0Count = _countof(kMelBlk0);
const uint8_t kMelBlk1Count = _countof(kMelBlk1);

/**
 *  boot    C D E F G A B C
 */
static const Note_t s_bootNotes[] = {
    { NOTE_C0,  NOTELEN8},
    { NOTE_D0,  NOTELEN8},
    { NOTE_E0,  NOTELEN8},
    { NOTE_F0,  NOTELEN8},
    { NOTE_G0,  NOTELEN8},
    { NOTE_A0,  NOTELEN8},
    { NOTE_B0,  NOTELEN8},
    { NOTE_C1,  NOTELEN8},
};

/**
 * fuga      G D Bb A G Bb A G F# A D
 * only play          G Bb A G F#
 */
static const Note_t s_fugaNotes[] = {
    { NOTE_G0,  NOTELEN8},
    { NOTE_B0b, NOTELEN8},
    { NOTE_A0,  NOTELEN8},
    { NOTE_G0,  NOTELEN8},
    { NOTE_F0s, NOTELEN8}
};

static const Note_t s_fugaExecutedHigh[] = {
    { NOTE_G0,  NOTELEN4},
    { NOTE_D1,  NOTELEN4},
    { NOTE_G1,  NOTELEN4}
};

static const Note_t s_fugaExecutedLow[] = {
    { NOTE_G1,  NOTELEN4},
    { NOTE_D1,  NOTELEN4},
    { NOTE_G0,  NOTELEN4},
};

/**
 * pic       G F Bb C F D C F D Bb C G F
 * only play          F D C F D Bb C
 */
static const Note_t s_picNotes[] = {
    { NOTE_F1,  NOTELEN8},
    { NOTE_D1,  NOTELEN4},
    { NOTE_C1,  NOTELEN8},
    { NOTE_F1,  NOTELEN8},
    { NOTE_D1,  NOTELEN4},
    { NOTE_B0b, NOTELEN4},
    { NOTE_C1,  NOTELEN4},
};

static const Note_t s_picExecuted[] = {
    { NOTE_F0,  NOTELEN4},
    { NOTE_A0, NOTELEN4},
    { NOTE_C1,  NOTELEN4},
    { NOTE_F1,  NOTELEN4},
};

/**
 * bethooben G G G Eb F F F D
 * only play          F F F   G G F Eb C G
 */
static const Note_t s_betNotes[] = {
    { NOTE_F1,  NOTELEN8},
    { NOTE_F1,  NOTELEN8},
    { NOTE_F1,  NOTELEN8},
};

static const Note_t s_betExecuted[] = {
    { NOTE_NONE,NOTELEN16},
    { NOTE_G0,  NOTELEN16},
    { NOTE_G0,  NOTELEN16},
    { NOTE_F0,  NOTELEN16},
    { NOTE_E0b, NOTELEN16},
    { NOTE_NONE,NOTELEN16},
    { NOTE_NONE,NOTELEN8},
    { NOTE_C0,  NOTELEN16},
    { NOTE_NONE,NOTELEN16},
    { NOTE_NONE,NOTELEN8},
    { NOTE_G0,  NOTELEN8},
};

/**
 * Elise      E Eb E Eb E B D C A
 * only play              B D C
 */
static const Note_t s_eliNotes[] = {
    { NOTE_NONE,NOTELEN16},
    { NOTE_B0,	NOTELEN16},
    { NOTE_D1,  NOTELEN16},
    { NOTE_C1,  NOTELEN16},
};

static const Note_t s_eliExecuted[] = {
    { NOTE_NONE,NOTELEN16},
    { NOTE_C0,  NOTELEN16},
    { NOTE_E0,  NOTELEN16},
    { NOTE_A0,  NOTELEN16},
    { NOTE_B0,  NOTELEN16},
    { NOTE_NONE,NOTELEN16},
    { NOTE_NONE,NOTELEN8},
    { NOTE_NONE,NOTELEN16},
    { NOTE_E0,  NOTELEN16},
    { NOTE_G0s, NOTELEN16},
    { NOTE_B0,  NOTELEN16},
    { NOTE_C1,  NOTELEN8},
};

/**
 * Dvorak      Eb F Gb F Eb Eb Eb C# Bb C# Eb
 * only play                Eb Eb C# Bb
 */
static const Note_t s_dovNotes[] = {
    { NOTE_E1b,  NOTELEN4},
    { NOTE_E1b,  NOTELEN4},
    { NOTE_C1s,  NOTELEN8},
    { NOTE_B0b,  NOTELEN16},
    /*{ NOTE_C1s,  NOTELEN16},
    { NOTE_E1b,  NOTELEN4},*/
};

static const Note_t s_dovExecuted[] = {
    { NOTE_E1b,  NOTELEN8},
    { NOTE_G1b,  NOTELEN6},
    { NOTE_E1b,  NOTELEN6},
    { NOTE_G1b,  NOTELEN6},
    { NOTE_B1b,  NOTELEN8},
    { NOTE_B0b,  NOTELEN8},
    { NOTE_E1b,  NOTELEN4},
};

/**
 * blk       F Bb A F D Eb D C Bb Bb F F Bb C Bb C
 * only play            Eb D C
 */
static const Note_t s_blkNotes[] = {
    { NOTE_E1b,  NOTELEN8},
    { NOTE_D1,   NOTELEN4},
    { NOTE_C1,   NOTELEN8},
};

static const Note_t s_blkExecuted[] = {
    { NOTE_NONE,NOTELEN8},
    { NOTE_F0,  NOTELEN8},
    { NOTE_F0,  NOTELEN4},
    { NOTE_B0b, NOTELEN4},
    { NOTE_C1,  NOTELEN4},
    { NOTE_B0b, NOTELEN4},
    { NOTE_C1,  NOTELEN1},
};

/**
 * melody when tuning is OK
 */
static const Note_t s_tuneGoodNotes[] = {
    { NOTE_SYM,  NOTELEN8},
};

/**
 * melody when tuning is too high
 */
static const Note_t s_tuneHighNotes[] = {
    { NOTE_TRI,  NOTELEN8},
};

/**
 * melody when tuning is too low
 */
static const Note_t s_tuneLowNotes[] = {
    { NOTE_BDRM,  NOTELEN8},
};

static const uint8_t kMelodyIdFuga              = 0;
static const uint8_t kMelodyIdTrp               = 1;
static const uint8_t kMelodyIdToggleGainHigh    = 2;
static const uint8_t kMelodyIdToggleGainLow     = 3;
static const uint8_t kMelodyIdInstChangeTrp     = 4;
static const uint8_t kMelodyIdBoot              = 5;
static const uint8_t kMelodyIdGood              = 6;
static const uint8_t kMelodyIdHigh              = 7;
static const uint8_t kMelodyIdLow               = 8;
static const uint8_t kMelodyIdBet               = 9;
static const uint8_t kMelodyIdToggleTuning      = 10;
static const uint8_t kMelodyIdPno               = 11;
static const uint8_t kMelodyIdInstChangePno     = 12;
static const uint8_t kMelodyIdOrc               = 13;
static const uint8_t kMelodyIdInstChangeOrc     = 14;
static const uint8_t kMelodyIdHmk               = 15;
static const uint8_t kMelodyIdInstChangeHmk     = 16;

/**
 * Melody table
 */
static const Melody_t s_melodies[] = {
    { CH_POR, s_fugaNotes,          _countof(s_fugaNotes)       },
    { CH_TRP, s_picNotes,           _countof(s_picNotes)        },
    { CH_POR, s_fugaExecutedHigh,   _countof(s_fugaExecutedHigh)},
    { CH_POR, s_fugaExecutedLow,    _countof(s_fugaExecutedLow) },
    { CH_POR, s_picExecuted,        _countof(s_picExecuted)     },
    { CH_PNO, s_bootNotes,          _countof(s_bootNotes)       },
    { CH_PCS, s_tuneGoodNotes,      _countof(s_tuneGoodNotes)   },
    { CH_PCS, s_tuneHighNotes,      _countof(s_tuneHighNotes)   },
    { CH_PCS, s_tuneLowNotes,       _countof(s_tuneLowNotes)    },
    { CH_STR, s_betNotes,           _countof(s_betNotes)        },
    { CH_STR, s_betExecuted,        _countof(s_betExecuted)     },
    { CH_PNO, s_eliNotes,           _countof(s_eliNotes)        },
    { CH_PNO, s_eliExecuted,        _countof(s_eliExecuted)     },
    { CH_BRA, s_dovNotes,           _countof(s_dovNotes)        },
    { CH_BRA, s_dovExecuted,        _countof(s_dovExecuted)     },
    { CH_HMK, s_blkNotes,        	_countof(s_blkNotes)     	},
    { CH_HMK, s_blkExecuted,        _countof(s_blkExecuted)     },
};

BleMidiCommunicator::BleMidiCommunicator()
:
m_ble(new Rn4020Controller(RN4020))
{
    memset(m_selected, kInvalidNote, sizeof(m_selected));
    memset(m_channels, 0, sizeof(m_channels));
    _gainVal = 1;
    m_isTuningMode = false;
    m_selected[CH_HMK] = 0;
}

/**
 *  never tested
 */
BleMidiCommunicator::~BleMidiCommunicator()
{
    delete m_ble;
}

uint32_t BleMidiCommunicator::Initialize()
{
    DEBUG("%s ENT", __FUNCTION__);
    
    int counter = 0;
    while(true) {
        if(initializeDevice()) {
            break;
        }
        delay(1000);
    }
	
	programChannel(CH_PNO, INST_PNO);
	programChannel(CH_POR, INST_POR);
	programChannel(CH_TRP, INST_TRP);
	programChannel(CH_PCS, INST_PCS);
	programChannel(CH_GTR, INST_GTR);
	programChannel(CH_VLN, INST_VLN);
	programChannel(CH_STR, INST_STR);
	programChannel(CH_STR, INST_STR);
	programChannel(CH_BRA, INST_BRA);
	programChannel(CH_TBA, INST_TBA);
	programChannel(CH_CTR, INST_CTR);
	programChannel(CH_OBE, INST_OBE);
	programChannel(CH_PCC, INST_PCC);
	
	setLylic(LYLIC);
	
	delay(1900);
	
	playMelody(kMelodyIdBoot);
	
    DEBUG("%s EXT", __FUNCTION__);
    return 0;
}

void BleMidiCommunicator::UpdateFreq(uint16_t freq, uint16_t note)
{
    DEBUG("%s ENT", __FUNCTION__);
    
    if(_pitchVal[1] == note) {
        DEBUG("%s EXT", __FUNCTION__);
        return;
    }
    
    if(_pitchVal[1] != 0) {
        DEBUG("off before on");
        userNoteOff(_pitchVal[1]);
    }
    
    if(note != 0) {
        DEBUG("sending on");
        userNoteOn(note);
    }

    // unused but update just for in case. _pitchVal[1] should be updated in noteOn/Off()
    _pitchVal[0] = freq;
    _pitchVal[1] = note;
    DEBUG("%s freq=%d, note=%d sent", __FUNCTION__, _pitchVal[0], _pitchVal[1]);
    
    DEBUG("%s EXT", __FUNCTION__);
}

uint8_t BleMidiCommunicator::GetGain()
{
    return _gainVal;
}

bool BleMidiCommunicator::IsTuningMode()
{
    return m_isTuningMode;
}

uint32_t BleMidiCommunicator::Yield()
{
    m_ble->Yield();
    return 0;
}

bool BleMidiCommunicator::initializeDevice()
{
    return m_ble->Initialize();
}

void BleMidiCommunicator::ConfirmaToggleGain()
{
    playMelody(kMelodyIdFuga);
}

void BleMidiCommunicator::ToggleGain()
{
    DEBUG("ToggleGain");
    if(_gainVal == 0) {
        // gain low -> hight
        playMelody(kMelodyIdToggleGainHigh);
        _gainVal = 1;
    } else {
        // gain high -> lo
        playMelody(kMelodyIdToggleGainLow);
        _gainVal = 0;
    }
}

void BleMidiCommunicator::ConfirmInstChange(uint8_t inst)
{
    userNoteOff(_pitchVal[1]);
    
    uint8_t melId = 0;
    switch(inst) {
        case kInstHmk:
            melId = kMelodyIdHmk;
            break;
        case kInstTrp:
            melId = kMelodyIdTrp;
            break;
        case kInstOrc:
            melId = kMelodyIdOrc;
            break;
        case kInstPno:
        default:
            melId = kMelodyIdPno;
            break;
    }
    DEBUG("ConfirmInstChange melody %d", melId);
    
    playMelody(melId);
}

void BleMidiCommunicator::ChangeInst(uint8_t inst)
{
    DEBUG("ChangeInst");

    userNoteOff(_pitchVal[1]);
    memset(m_selected, kInvalidNote, sizeof(m_selected));
    
    uint8_t melId = 0;
    switch(inst) {
        case kInstHmk:
            melId = kMelodyIdInstChangeHmk;
            m_selected[CH_HMK] = 0;
            break;
        case kInstTrp:
            melId = kMelodyIdInstChangeTrp;
            m_selected[CH_TRP] = 0;
            break;
        case kInstOrc:
            melId = kMelodyIdInstChangeOrc;
            m_selected[CH_TBA] = -29;
            m_selected[CH_CTR] = -20;
            m_selected[CH_BRA] = -5;
            m_selected[CH_STR] = 4;
            m_selected[CH_OBE] = 7;
            m_selected[CH_PCC] = 12;
            break;
        case kInstPno:
        default:
            melId = kMelodyIdInstChangePno;
            m_selected[CH_PNO] = 0;
            break;
    }
    DEBUG("ChangeInst melody %d", melId);
    
    playMelody(melId);
}

void BleMidiCommunicator::ConfirmToggleTuning()
{
    playMelody(kMelodyIdBet);
}

void BleMidiCommunicator::ToggleTuning()
{
    DEBUG("ToggleTuning");
    userNoteOff(_pitchVal[1]);
    m_isTuningMode = !m_isTuningMode;
    playMelody(kMelodyIdToggleTuning);
}

void BleMidiCommunicator::NotifyTune(uint8_t tune)
{
    switch(tune) {
        case 1:
            playMelody(kMelodyIdGood);
            break;
        case 2:
            playMelody(kMelodyIdHigh);
            break;
        case 3:
            playMelody(kMelodyIdLow);
            break;
        default:
            break;
    }
}

void BleMidiCommunicator::userNoteOff(uint16_t note)
{
    for(int8_t ch = 0; ch < 16; ch++) {
    	if(m_selected[ch] == kInvalidNote) {
    		continue;
    	}
    	noteOff(ch, note + m_selected[ch]);
    }
}

void BleMidiCommunicator::userNoteOn(uint16_t note)
{
    for(int8_t ch = 0; ch < 16; ch++) {
    	if(m_selected[ch] == kInvalidNote) {
    		continue;
    	}
    	noteOn(ch, note + m_selected[ch]);
    }
}

void BleMidiCommunicator::noteOff(uint8_t ch, uint16_t note)
{
    char cmd[10]  = {0};
    cmd[0] = '8'; cmd[1] = '0'; cmd[2] = '8'; cmd[3] = '0';
    
    cmd[4] = '8';
    itoaUi8(ch, &(cmd[5]));
    itoaUi16(note, &(cmd[6]));
    cmd[8] = '7';
    cmd[9] = 'F';
    
    DEBUG(cmd);
    m_ble->WriteCharacteristic(cmd, 10);
    m_channels[ch] = 0;
}

void BleMidiCommunicator::noteOn(uint8_t ch, uint16_t note)
{
    char cmd[10]  = {0};
    cmd[0] = '8'; cmd[1] = '0'; cmd[2] = '8'; cmd[3] = '0';
    
    cmd[4] = '9';
    itoaUi8(ch, &(cmd[5]));
    itoaUi16(note, &(cmd[6]));
    cmd[8] = '7';
    cmd[9] = 'F';
    
    m_ble->WriteCharacteristic(cmd, 10);
    m_channels[ch] = note;
}

void BleMidiCommunicator::playMelody(uint8_t id)
{
    const Melody_t* mel = &(s_melodies[id]);
    uint8_t ch = mel->ch;
    
    if(m_channels[ch] != 0) {
        noteOff(ch, m_channels[ch]);
    }
    
    for(int i = 0; i < mel->note_len; i++) {
        const Note_t* playNote = &mel->playNotes[i];
        if(playNote != NOTE_NONE) {
            noteOn(ch, playNote->note);
        }
        delay(playNote->delay);
        noteOff(ch, playNote->note);
    }
}

void BleMidiCommunicator::programChannel(uint8_t ch, uint8_t inst)
{
    char cmd[8]  = {0};
    cmd[0] = '8'; cmd[1] = '0'; cmd[2] = '8'; cmd[3] = '0';
    
    cmd[4] = 'C';
    itoaUi8(ch, &(cmd[5]));
    itoaUi16(inst, &(cmd[6]));
    
    m_ble->WriteCharacteristic(cmd, 8);
}

void BleMidiCommunicator::setLylic(uint8_t lylic)
{
	char cmd[32] = {0};
	strncpy(cmd, SYSEX_TEMPLATE, 24);
	itoaUi16(lylic, &cmd[18]);
	
	m_ble->WriteCharacteristic(cmd, 24);
}
