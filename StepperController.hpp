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

    // Must be called periodically (ideally fixed rate) to update the pulse output
    void pulseUpdate();

    void setDirection(bool up);

    void enableFor(unsigned long milliseconds);

    void enable(bool enable);
    bool isEnabled();

private:
    int _pulsePin;
    int _directionPin;
    int _enablePin;

    unsigned long _lastUpdate;
    unsigned long _updatePeriodUs;
    unsigned long _disableMillis;

    bool _directionUp;
    bool _pulsing;
    bool _enabled;
};

#endif // STEPPER_CONTROLLER_HPP__
