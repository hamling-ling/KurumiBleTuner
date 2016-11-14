#include "Communicator.h"
#include <Arduino.h>

Communicator::Communicator()
{
    memset(_pitchVal, 0, sizeof(_pitchVal));
    _gainVal = 0;
}

Communicator::~Communicator()
{
}

uint32_t Communicator::Initialize()
{
    return 0;
}

uint32_t Communicator::Yield()
{
    return 0;
}

void Communicator::UpdateFreq(uint16_t freq, uint16_t note)
{
}

uint8_t Communicator::GetGain()
{
    return 0;
}

bool Communicator::IsTuningMode()
{
    return false;
}

void Communicator::ConfirmaToggleGain()
{
}

void Communicator::ToggleGain()
{
}

void Communicator::ConfirmInstChange(uint8_t inst)
{
}

void Communicator::ChangeInst(uint8_t inst)
{
}

void Communicator::ConfirmToggleTuning()
{
}

void Communicator::ToggleTuning()
{
}

void Communicator::NotifyTune(uint8_t tune)
{
}


