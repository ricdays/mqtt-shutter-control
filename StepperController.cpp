#include "StepperController.hpp"

#include "Arduino.h"

namespace
{
const int kDefaultUpdatePeriodUs = 500;
}

StepperController::StepperController(int pulsePin, int directionPin, int enablePin)
    : _pulsePin(pulsePin)
    , _directionPin(directionPin)
    , _enablePin(enablePin)
    , _updatePeriodUs(kDefaultUpdatePeriodUs)
    , _directionUp(true)
    , _pulsing(false)
    , _enabled(false)
{
    _lastUpdate = micros();
}

void StepperController::setDirection(bool up)
{
    _directionUp = up;
}

void StepperController::enable(bool enable)
{
    _enabled = enable;
}

bool StepperController::isEnabled()
{
    return _enabled;
}

void StepperController::setup()
{
    // Set pins as output
    pinMode(_pulsePin, OUTPUT);
    pinMode(_directionPin, OUTPUT);
}

void StepperController::refresh()
{
    unsigned long currentTime = micros();
    if (currentTime - _lastUpdate < _updatePeriodUs)
        return;

    _lastUpdate = currentTime;

    digitalWrite(_enablePin, _enabled ? LOW : HIGH);
    if (!_enabled)
        return;

    if (!_pulsing)
    {
        digitalWrite(_directionPin, _directionUp ? HIGH : LOW);
        digitalWrite(_pulsePin, LOW);
        _pulsing = true;
    }
    else
    {
        digitalWrite(_pulsePin, HIGH);
        _pulsing = false;
    }
}
