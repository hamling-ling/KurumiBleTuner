#ifndef _COMMUNICATOR_H_
#define _COMMUNICATOR_H_

#include <inttypes.h>

class Communicator
{
public:
    Communicator();
    virtual ~Communicator();
    
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
    
protected:
    uint16_t _pitchVal[2];
    uint8_t _gainVal;
};

#endif //_COMMUNICATOR_H_
