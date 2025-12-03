#ifndef MQTT_UTILS_H
#define MQTT_UTILS_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define BLIND_NO 2

// MQTT Configuration
String mqttServer = "homeassistant.local";
const int mqttPort = 1883;
String mqttUsername = "mintek_blinds";
String mqttPassword = "123";
String mqttClientId = "mintek_blinds_" + String(BLIND_NO);

// Home Assistant MQTT Configuration
String discoveryTopic = "homeassistant/cover/" + mqttClientId + "/config";
String commandTopic = mqttClientId + "/set";
String positionTopic = mqttClientId + "/position"; // Used for reporting current state (0-100)
String setPositionTopic = mqttClientId + "/set_position"; // Used for setting the position (0-100)
String availabilityTopic = mqttClientId + "/availability";

// MQTT Payloads
const char* payloadAvailable = "online";
const char* payloadNotAvailable = "offline";
const char* payloadOpen = "OPEN";
const char* payloadClose = "CLOSE";
const char* payloadStop = "STOP";

// MQTT State Variables
bool mqttSetupActive = false;
bool mqttAvailableMsgSent = false;
bool mqttDiscoveryMsgSent = false;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void checkMQTTCallBack(char* topic, byte* payload, unsigned int length) {
    printSeparator(1);
    Serial.println("MQTT Message received");
    Serial.println("Topic: " + String(topic));
    // Properly create string from payload using the length parameter
    String payloadStr;
    payloadStr.reserve(length);
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    Serial.println("Payload: " + payloadStr);
    Serial.println("Length: " + String(length));
    printSeparator(3);
}

void setupMQTT() {
    if (mqttSetupActive) return;
    setLedColor(128, 0, 128); // purple
    printSeparator(1);
    Serial.println("Connecting to MQTT...");

    int attempt = 0;
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    while (!mqttClient.connect(mqttClientId.c_str(), mqttUsername.c_str(), mqttPassword.c_str()) && attempt < 10) {
        Serial.print(".");
        delay(5000);
        attempt++;
    }
    if (mqttClient.connect(mqttClientId.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) {
        Serial.println("Connected");
        mqttClient.setCallback(checkMQTTCallBack);
        mqttSetupActive = true;
    }
    else{
        Serial.println("Failed to connect to MQTT after " + String(attempt) + " attempts");
        setLedColor(0, 255, 0); // red
    }
    delay(1000);
    setLedOff();
    printSeparator(3);
}

// source: https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery
// source: https://www.home-assistant.io/integrations/cover.mqtt/
void sendMQTTDiscoveryMessage() {
    if (mqttDiscoveryMsgSent) return;
    printSeparator(1);
    Serial.println("Sending MQTT Discovery Message...");

    DynamicJsonDocument doc(1024);
    bool retain = true;

    //basic config
    doc["name"] = "ESP8266_" + mqttClientId;
    doc["unique_id"] = mqttClientId;
    doc["qos"] = 0;
    doc["retain"] = false;
    doc["optimistic"] = false;
  
    // Topics
    doc["command_topic"] = commandTopic;
    doc["position_topic"] = positionTopic;
    doc["set_position_topic"] = setPositionTopic;
    doc["availability_topic"] = availabilityTopic;
  
    // Payloads
    doc["payload_open"] = payloadOpen;
    doc["payload_close"] = payloadClose;
    doc["payload_stop"] = payloadStop;
    doc["payload_available"] = payloadAvailable;
    doc["payload_not_available"] = payloadNotAvailable;
    
    // Position Mapping
    doc["position_open"] = 100;
    doc["position_closed"] = 0;
  
    // Device Info (Optional but Recommended)
    JsonObject device = doc.createNestedObject("device");
    device["identifiers"] = mqttClientId + String("_identifier");
    device["name"] = mqttClientId + String("_device");
    device["manufacturer"] = "DIY";
    device["icon"] = "mdi:blinds";
  
    // String payload;
    // serializeJson(doc, payload);
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
  
    // mqttDiscoveryMsgSent = mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), retain);
    mqttDiscoveryMsgSent = mqttClient.publish(discoveryTopic.c_str(), (const uint8_t*)buffer, n, retain);
    if (mqttDiscoveryMsgSent)
        Serial.println("Discovery message published successfully");
    else
        Serial.println("ERROR: Failed to publish discovery message");

    mqttClient.subscribe(discoveryTopic.c_str());
    mqttClient.subscribe(commandTopic.c_str());
    mqttClient.subscribe(setPositionTopic.c_str());
    mqttClient.subscribe(availabilityTopic.c_str());
    mqttClient.subscribe(positionTopic.c_str());
    delay(500);
    printSeparator(3);
}

void sendMQTTAvailabilityMessage() {
    if (mqttAvailableMsgSent) return;
    printSeparator(1);
    Serial.println("Sending MQTT Availability Message...");
    mqttAvailableMsgSent = mqttClient.publish(availabilityTopic.c_str(), payloadAvailable);
    if (mqttAvailableMsgSent)
        Serial.println("Availability message published successfully");
    else
        Serial.println("ERROR: Failed to publish availability message");
    printSeparator(3);
}

void handleMQTTServer() {
    mqttClient.loop();
}

#endif // MQTT_UTILS_H