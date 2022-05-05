#ifndef STEPPER_CONTROLLER_HPP__
#define STEPPER_CONTROLLER_HPP__

/**
 * 
 * */

class StepperController
{
public:
    StepperController(int pulsePin, int directionPin, int enablePin);

    // Must be called during setup to configure output pins
    void setup();

    // Must be called periodically to update the output
    void refresh();

    void setDirection(bool up);

    void enable(bool enable);
    bool isEnabled();

private:
    int _pulsePin;
    int _directionPin;
    int _enablePin;

    unsigned long _lastUpdate;
    unsigned long _updatePeriodUs;

    bool _directionUp;
    bool _pulsing;
    bool _enabled;
};

#endif // STEPPER_CONTROLLER_HPP__
