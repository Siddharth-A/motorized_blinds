/*
ESP8266 AP Mode - WiFi Configuration Portal
*/

#include <EEPROM.h>

#include "led_utils.h"
#include "mqtt_utils.h"
#include "wifi_utils.h"
#include "eeprom_utils.h"

// Define the LED array (declared as extern in led_utils.h)
CRGB leds[NEOPIXEL_COUNT];

// Wifi Button Configuration
#define WIFI_SETUP_BUTTON 4
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // 50ms debounce delay

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n \n \n");

    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);
    // clearEEPROM();

    // Initialize LED
    initLed();

    // Initialize WIFI_SETUP_BUTTON
    pinMode(WIFI_SETUP_BUTTON, INPUT_PULLUP);
    lastButtonState = digitalRead(WIFI_SETUP_BUTTON);
    currentButtonState = lastButtonState;

    // Check if WiFi credentials are already stored
    if (readWifiCredentialsFromEEPROM())
      connectToWiFi();
    else{
      setupWifi();
      connectToWiFi();
    }
}

void loop() {
    connectToWiFi();
    handleWiFiServer();
  
    if (getWifiStatus()) {
        setupMQTT();
        sendMQTTDiscoveryMessage();
        sendMQTTAvailabilityMessage();
    }
    handleMQTTServer();

    // Check Wifi Setup Button
    int reading = digitalRead(WIFI_SETUP_BUTTON);
    if (reading != lastButtonState) lastDebounceTime = millis();
    if ((millis() - lastDebounceTime) > debounceDelay && reading != currentButtonState) {
        currentButtonState = reading;
        if (currentButtonState == LOW){
          resetWifiSetup();
          setupWifi();
        }
    }
    lastButtonState = reading;
}
