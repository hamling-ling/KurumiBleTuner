#ifndef _BLECOMMUNICATOR_H_
#define _BLECOMMUNICATOR_H_

#include "Communicator.h"

class BleCommunicator : public Communicator
{
public:
    BleCommunicator();
    ~BleCommunicator();
    
    virtual uint32_t Initialize();
    virtual uint32_t Yield();
    virtual void UpdateFreq(uint16_t freq, uint16_t note);
    virtual uint8_t GetGain();

private:

    void recvCommand();
    bool initializeDevice();
    void purge();
};

#endif //_BLECOMMUNICATOR_H_
