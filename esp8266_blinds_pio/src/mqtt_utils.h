#ifndef MQTT_UTILS_H
#define MQTT_UTILS_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define BLIND_NO 1

// MQTT Configuration
String mqttServer = "homeassistant.local";
const int mqttPort = 1883;
String mqttUsername = "mintek_blinds";
String mqttPassword = "123";
String mqttClientId = "mintek_blinds_" + String(BLIND_NO);
// String blindTopic = "homeassistant/sensor/blind_" + String(BLIND_NO);

// Home Assistant MQTT Configuration
String discoveryPrefix = "homeassistant";
String component = "cover";
String nodeId = mqttClientId;

String discoveryTopic = discoveryPrefix + "/" + component + "/" + nodeId + "/config";
String stateTopic = component + "/" + nodeId + "/state";
String availabilityTopic = component + "/" + nodeId + "/availability";

// MQTT State Variables
bool mqttSetupActive = false;
bool mqttAvailableMsgSent = false;
bool mqttDiscoveryMsgSent = false;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void checkMQTTCallBack(char* topic, byte* payload, unsigned int length) {
    Serial.println("MQTT Message received");
    Serial.println("Topic: " + String(topic));
    Serial.println("Payload: " + String((char*)payload));
    Serial.println("Length: " + String(length));
}

void setupMQTT() {
    if (mqttSetupActive) return;
    setLedColor(128, 0, 128); // purple
    Serial.println("\n \n \n");
    Serial.println("----------------------------------------------------------------");
    Serial.println("Connecting to MQTT...");

    int attempt = 0;
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    while (!mqttClient.connect(mqttClientId.c_str(), mqttUsername.c_str(), mqttPassword.c_str()) && attempt < 10) {
        Serial.print(".");
        delay(5000);
        attempt++;
    }
    Serial.println("\n");
    if (mqttClient.connect(mqttClientId.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) {
        Serial.println("Connected to MQTT");
        mqttClient.setCallback(checkMQTTCallBack);
        setLedOff();
        mqttSetupActive = true;
    }
    else{
        Serial.println("Failed to connect to MQTT after " + String(attempt) + " attempts");
        setLedColor(0, 255, 0); // red
    }
    Serial.println("----------------------------------------------------------------");
}

// source: https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery
void sendMQTTDiscoveryMessage() {
    if (mqttDiscoveryMsgSent) return;
    setLedColor(255, 255, 0); // yellow
    Serial.println("\n \n \n");
    Serial.println("----------------------------------------------------------------");
    Serial.println("Sending MQTT Discovery Message...");

    DynamicJsonDocument doc(1024);
    char buffer[256];
    bool retain = true;

    doc["name"] = mqttClientId;
    doc["state_topic"]   = stateTopic;
    doc["availability_topic"]   = availabilityTopic;
    doc["unique_id"] = mqttClientId + String("unique");
    size_t n = serializeJson(doc, buffer);
    mqttClient.publish(discoveryTopic.c_str(), (const uint8_t*)buffer, n, retain);
    mqttClient.subscribe(stateTopic.c_str());
    delay(500);
    setLedOff();
    mqttDiscoveryMsgSent = true;
    Serial.println("----------------------------------------------------------------");
}

void sendMQTTAvailabilityMessage() {
    if (mqttAvailableMsgSent) return;
    Serial.println("\n \n \n");
    Serial.println("----------------------------------------------------------------");
    Serial.println("Sending MQTT Availability Message...");
    String availability = "online";
    mqttClient.publish(availabilityTopic.c_str(), availability.c_str());
    mqttClient.subscribe(stateTopic.c_str());
    mqttAvailableMsgSent = true;
    Serial.println("----------------------------------------------------------------");
}

void handleMQTTServer() {
    mqttClient.loop();
}

#endif // MQTT_UTILS_H