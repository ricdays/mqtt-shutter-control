#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "WifiConfig.h"

#include "StepperController.hpp"

#define MOTOR1_PUL_PIN D0
#define MOTOR1_DIR_PIN D1
#define MOTOR1_ENA_PIN D2

WiFiClient espClient;
PubSubClient mqttClient(espClient);

StepperController motor1(MOTOR1_PUL_PIN, MOTOR1_DIR_PIN, MOTOR1_ENA_PIN);

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

void mqttCallback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    /*
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
        digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
        // but actually the LED is on; this is because
        // it is active low on the ESP-01)
    } else {
        digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    }
    */
    static bool ledStatus = false;
    ledStatus = !ledStatus;
    digitalWrite(LED_BUILTIN, ledStatus ? HIGH : LOW);

    String topicString(topic);

    const String topicPrefix = "studio/shutter/";

    if (!topicString.startsWith(topicPrefix))
        return;

    //Serial.println("TOPIC PREFIX OK");

    topicString = topicString.substring(topicPrefix.length());
    int slashIndex = topicString.indexOf('/');
    if (slashIndex < 1)
        return;

    String shutterId = topicString.substring(0, slashIndex);

    //Serial.print("Shutter ID: ");
    //Serial.println(shutterId);

    int motorIndex = shutterId[0] - '0';

    topicString = topicString.substring(shutterId.length() + 1);

    

    /*
    if (topicString.equals("studio/shutter/0/up"))
    {

    }
    */

}


void checkMqttConnection() {
    if (mqttClient.connected()) {
        return;
    }

    // Loop until we're reconnected
    while (!mqttClient.connected()) {
        
        Serial.print("Attempting MQTT connection...");
        
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);

        // Attempt to connect
        if (mqttClient.connect(clientId.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            mqttClient.publish("outTopic", "hello world");
            // ... and resubscribe
            mqttClient.subscribe("studio/shutter/0/up");
            mqttClient.subscribe("studio/shutter/0/down");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Booting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
        else // U_SPIFFS
        type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    mqttClient.setServer(mqtt_server, 1883);
    mqttClient.setCallback(mqttCallback);

    motor1.setup();
}

unsigned long lastChange = millis();
unsigned long lastEnable = millis();
bool directionUp = true;

void loop() {
    checkWifiConnection();
    checkMqttConnection();
    ArduinoOTA.handle();
    mqttClient.loop();

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
