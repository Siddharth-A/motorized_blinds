#ifndef MQTT_UTILS_H
#define MQTT_UTILS_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define BLIND_NO 1
#define BLIND_NAME "Family Room Blinds"

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

    // Increase buffer size to handle larger messages
    mqttClient.setBufferSize(512);

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

    mqttClient.subscribe(discoveryTopic.c_str());
    mqttClient.subscribe(commandTopic.c_str());
    mqttClient.subscribe(setPositionTopic.c_str());
    mqttClient.subscribe(availabilityTopic.c_str());
    mqttClient.subscribe(positionTopic.c_str());

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
    char buffer[512];
    bool retain = true;

    // Basic config with abbreviations to save memory
    doc["name"] = "ESP8266_" + mqttClientId;
    doc["uniq_id"] = mqttClientId;      // abbreviated: uniq_id
    doc["qos"] = 0;
    doc["retain"] = retain;
    doc["optimistic"] = false;

    // Topics (using abbreviations)
    doc["cmd_t"] = commandTopic;            // abbreviated: command_topic
    doc["pos_t"] = positionTopic;           // abbreviated: position_topic
    doc["set_pos_t"] = setPositionTopic;    // abbreviated: set_pos_t
    doc["avty_t"] = availabilityTopic;      // abbreviated: avty_t

    // Payloads (using abbreviations)
    doc["pl_open"] = payloadOpen;               // abbreviated: pl_open
    doc["pl_cls"] = payloadClose;               // abbreviated: pl_cls
    doc["pl_stop"] = payloadStop;               // abbreviated: pl_stop
    doc["pl_avail"] = payloadAvailable;         // abbreviated: pl_avail
    doc["pl_not_avail"] = payloadNotAvailable;  // abbreviated: pl_not_avail

    // Position Mapping
    doc["pos_open"] = 100;  // abbreviated: position_open
    doc["pos_closed"] = 0;  // abbreviated: position_closed

    // Device Info (Required for device-based discovery)
    JsonObject device = doc.createNestedObject("dev");      // abbreviated: device
    device["ids"] = mqttClientId;                          // abbreviated: ids (can be string or array)
    device["name"] = String(BLIND_NAME);
    device["mf"] = "Mintek";                               // abbreviated: mf
    device["mdl"] = "";                                    // abbreviated: mdl
    device["sw"] = "";                                     // abbreviated: sw

    // Origin Info (Recommended/Required for device-based discovery)
    JsonObject origin = doc.createNestedObject("o");        // abbreviated: origin
    origin["name"] = String(BLIND_NAME);
    origin["sw"] = "";                                     // abbreviated: sw

    size_t n = serializeJson(doc, buffer);
    Serial.println("Discovery message size: " + String(n));

    // mqttDiscoveryMsgSent = mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), retain);
    mqttDiscoveryMsgSent = mqttClient.publish(discoveryTopic.c_str(), (const uint8_t*)buffer, n, retain);
    if (mqttDiscoveryMsgSent)
        Serial.println("Discovery message published successfully");
    else
        Serial.println("ERROR: Failed to publish discovery message");

    delay(500);
    printSeparator(3);
}

void deleteMQTTDevice() {
    printSeparator(1);
    Serial.println("Deleting MQTT Device from Home Assistant...");
    
    // Publish empty payload to discovery topic with retain flag
    // This will remove the component and clear the published discovery payload
    bool success = mqttClient.publish(discoveryTopic.c_str(), "", true);
    
    if (success) {
        Serial.println("Device deletion message published successfully");
        Serial.println("Discovery topic: " + discoveryTopic);
        // Reset the discovery message sent flag so it can be re-sent if needed
        mqttDiscoveryMsgSent = false;
    } else {
        Serial.println("ERROR: Failed to publish device deletion message");
    }
    
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