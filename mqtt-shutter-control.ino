#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WifiConfig.h"

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
}

unsigned long lastKeepAlive = millis();
bool directionUp = true;

void loop() {
    checkWifiConnection();
    OTA_loopStep();
    MQTT_loopStep();
    motor1.refresh();
}
