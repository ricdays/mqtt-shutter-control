#ifndef MQTT_HPP__
#define MQTT_HPP__

#include "Arduino.h"

void MQTT_setup();
void MQTT_loopStep();
void MQTT_callback(char* topic, byte* payload, unsigned int length);

#endif // MQTT_HPP__
