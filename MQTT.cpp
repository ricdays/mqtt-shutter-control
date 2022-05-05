#include "MQTT.hpp"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "StepperController.hpp"

extern PubSubClient mqttClient;
extern StaticJsonDocument<200> doc;
extern StepperController* motors[];
extern int numMotors;
extern const char* CONFIG_MQTT_SERVER;

void doActionStop(int motorIndex)
{
    motors[motorIndex]->enable(false);
}

void doActionMove(int motorIndex)
{
    if (!doc.containsKey("dir"))
    {
        Serial.println(F("Missing dir"));
        return;
    }

    String direction = doc["dir"].as<String>();
    if (direction.equals("up"))
    {
        motors[motorIndex]->setDirection(true);
    }
    else if (direction.equals("down"))
    {
        motors[motorIndex]->setDirection(false);
    }
    else
    {
        Serial.println(F("Invalid dir"));
        return;
    }

    unsigned long holdTime = 100; // Optional
    if (doc.containsKey("hold_time"))
    {
        holdTime = doc["hold_time"].as<unsigned long>();
    }

    motors[motorIndex]->enableFor(holdTime);
    //Serial.println(F("MOVING!"));

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
            mqttClient.publish("studio/shutter", "hello");

            // ... and resubscribe
            mqttClient.subscribe("studio/shutter/0");
            mqttClient.subscribe("studio/shutter/1");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void MQTT_setup()
{
    mqttClient.setServer(CONFIG_MQTT_SERVER, 1883);
    mqttClient.setCallback(MQTT_callback);
}

void MQTT_loopStep()
{
    checkMqttConnection();
    mqttClient.loop();

    static unsigned long lastKeepAlive = millis();
    unsigned long current = millis();
    if (current - lastKeepAlive > 4000)
    {
        lastKeepAlive = current;
        if (mqttClient.connected())
        {
            mqttClient.publish("studio/shutter", "keepalive");
        }
    }
}

void MQTT_callback(char* topic, byte* payload, unsigned int length) {

    /*
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    */

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
    int motorIndex = topicString[0] - '0';

    //Serial.print("Shutter ID: ");
    //Serial.println(motorIndex);

    //topicString = topicString.substring(shutterId.length() + 1);
    
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, payload, length);

    // Test if parsing succeeds.
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    if (!doc.containsKey("action"))
    {
        Serial.println(F("Missing action"));
        return;
    }

    String action = doc["action"].as<String>();
    if (action.equals("move"))
        doActionMove(motorIndex);
    else if (action.equals("stop"))
        doActionStop(motorIndex);
    else
        Serial.println(F("Invalid action"));

}