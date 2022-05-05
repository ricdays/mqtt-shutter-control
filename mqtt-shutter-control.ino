#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266Timer.h>
#include "Config.h"

#include "StepperController.hpp"
#include "OTA.hpp"
#include "MQTT.hpp"

#define MOTOR1_PUL_PIN D0
#define MOTOR1_DIR_PIN D1
#define MOTOR1_ENA_PIN D2

WiFiClient espClient;
PubSubClient mqttClient(espClient);
StaticJsonDocument<200> doc;

StepperController motor1(MOTOR1_PUL_PIN, MOTOR1_DIR_PIN, MOTOR1_ENA_PIN);
//StepperController motor2(MOTOR2_PUL_PIN, MOTOR2_DIR_PIN, MOTOR2_ENA_PIN);

StepperController* motors[] = { &motor1 };
int numMotors = sizeof(motors) / sizeof(StepperController*);

// Select a Timer Clock
#define USING_TIM_DIV1                false           // for shortest and most accurate timer
#define USING_TIM_DIV16               false           // for medium time and medium accurate timer
#define USING_TIM_DIV256              true            // for longest timer but least accurate. Default

// Init ESP8266 only and only Timer 1
ESP8266Timer ITimer;

void IRAM_ATTR TimerHandler()
{
    for (int i = 0; i < numMotors; ++i)
        motors[i]->pulseUpdate();
}

unsigned long lastWifiChecked = 0;
void checkWifiConnection()
{
    unsigned long currentMillis = millis();
    // if WiFi is down, try reconnecting
    if ((WiFi.status() != WL_CONNECTED) && 
        (lastWifiChecked == 0 || (currentMillis - lastWifiChecked >= 10000))
    ) {
        Serial.print(millis());
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        /*
        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
            Serial.println("Connection Failed! Rebooting...");
            delay(5000);
            ESP.restart();
        }*/
        lastWifiChecked = currentMillis;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Booting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    MQTT_setup();

    OTA_setup();
    
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    motor1.setup();

    // Configure interrupt timer - interval in microsecs
    if (ITimer.attachInterruptInterval(500, TimerHandler))
        Serial.print(F("Starting  ITimer OK"));
    else
        Serial.println(F("Can't set ITimer correctly. Select another freq. or interval"));
}

unsigned long lastKeepAlive = millis();
bool directionUp = true;

void loop() {
    checkWifiConnection();
    OTA_loopStep();
    MQTT_loopStep();

    for (int i = 0; i < numMotors; ++i)
        motors[i]->refresh();
}
