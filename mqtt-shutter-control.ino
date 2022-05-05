#include "StepperController.hpp"

#define MOTOR1_PUL_PIN D0
#define MOTOR1_DIR_PIN D1
#define MOTOR1_ENA_PIN D2

StepperController motor1(MOTOR1_PUL_PIN, MOTOR1_DIR_PIN, MOTOR1_ENA_PIN);

void setup() {
  motor1.setup();
}

unsigned long lastChange = millis();
unsigned long lastEnable = millis();
bool directionUp = true;

void loop() {
    motor1.refresh();


    unsigned long current = millis();
    if (current - lastChange > 2000)
    {
        lastChange = current;
        directionUp = !directionUp;
        motor1.setDirection(directionUp);
    }

    if (current - lastEnable > 5000)
    {
        lastEnable = current;
        motor1.enable(!motor1.isEnabled());
    }
}
