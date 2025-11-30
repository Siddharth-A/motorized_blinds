/*
ESP8266 AP Mode - WiFi Configuration Portal
*/


#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "wifi-info.h"

// AP Configuration
const char* ap_ssid = "Mintek_Blinds";
IPAddress ap_ip(192, 168, 1, 1);       // Static IP address for AP
IPAddress ap_gateway(192, 168, 1, 1);  // Gateway IP (usually same as AP IP)
IPAddress ap_subnet(255, 255, 255, 0); // Subnet mask

// Web Server
ESP8266WebServer server(80);

// EEPROM Configuration
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASSWORD_ADDR 64
#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 64

// Wifi Button Configuration
#define WIFI_SETUP_BUTTON 4
bool wifiSetupActive = false;
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // 50ms debounce delay


// Handle root request - serve the HTML page
void handleRoot(){
  String s = WIFI_INFO;
  server.send(200,"text/html",s);
}

void writeStringToEEPROM(int address, String data) {
    int len = data.length();
    EEPROM.write(address, len);
    for (int i = 0; i < len; i++) {
        EEPROM.write(address + 1 + i, data[i]);
    }
    EEPROM.commit();
}

String readStringFromEEPROM(int address) {
    int len = EEPROM.read(address);
    // Validate length (should be 0-64 for our use case)
    if (len <= 0 || len > 64) {
        return "";
    }
    String data = "";
    for (int i = 0; i < len; i++) {
        data += char(EEPROM.read(address + 1 + i));
    }
    return data;
}

void clearEEPROM() {
    // Write empty strings (length 0) to both locations
    EEPROM.write(SSID_ADDR, 0);
    EEPROM.write(PASSWORD_ADDR, 0);

    // Clear the data areas by writing 0 to all bytes
    for (int i = 1; i <= MAX_SSID_LEN; i++) {
        EEPROM.write(SSID_ADDR + i, 0);
    }
    for (int i = 1; i <= MAX_PASSWORD_LEN; i++) {
        EEPROM.write(PASSWORD_ADDR + i, 0);
    }

    EEPROM.commit();
    Serial.println("EEPROM cleared: SSID and password fields erased");
}

// Handle WiFi configuration POST request
void handleWiFiConfig() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");

        // Limit length to prevent overflow
        if (ssid.length() > MAX_SSID_LEN) ssid = ssid.substring(0, MAX_SSID_LEN);
        if (password.length() > MAX_PASSWORD_LEN) password = password.substring(0, MAX_PASSWORD_LEN);

        // Store credentials in EEPROM
        writeStringToEEPROM(SSID_ADDR, ssid);
        writeStringToEEPROM(PASSWORD_ADDR, password);

        Serial.println("WiFi credentials saved:");
        Serial.println("SSID: " + ssid);
        Serial.println("Password: " + password);

        // Send success response
        server.send(200, "text/plain", "WiFi credentials saved successfully!");
    } else {
        server.send(400, "text/plain", "Missing SSID or password");
    }
}

// Handle EEPROM clear request
void handleClearEEPROM() {
    clearEEPROM();
    server.send(200, "text/plain", "EEPROM cleared successfully! SSID and password fields erased.");
}

// Handle 404 - Not Found
void handleNotFound() {
    server.send(404, "text/plain", "Not Found");
}

// Setup Passwordless Access Point to intake WIFI credentials
void setupWifi() {
    // Only set up if not already active
    if (wifiSetupActive) {
        return;
    }
    Serial.println("\n \n \n");
    Serial.println("Starting WiFi Configuration Portal...");

    // Set up Access Point
    WiFi.mode(WIFI_AP);

    // Configure static IP for Soft AP
    WiFi.softAPConfig(ap_ip, ap_gateway, ap_subnet);

    // Start Access Point
    WiFi.softAP(ap_ssid, "");

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.print("AP SSID: ");
    Serial.println(ap_ssid);
    Serial.println("Connect to this network and navigate to http://" + IP.toString());

    // Set up web server routes (only register once)
    server.on("/", handleRoot);
    server.on("/wifi-config", HTTP_POST, handleWiFiConfig);
    server.on("/clear-eeprom", HTTP_GET, handleClearEEPROM);  // Clear EEPROM via GET request
    server.onNotFound(handleNotFound);

    // Start server
    server.begin();
    Serial.println("HTTP server started");

    wifiSetupActive = true;

    // Close the Access Point
    delay(1000);
    WiFi.softAPdisconnect(true);
}

void readWifiFromEEPROM() {
    String savedSSID = readStringFromEEPROM(SSID_ADDR);
    String savedPassword = readStringFromEEPROM(PASSWORD_ADDR);

    if (savedSSID.length() > 0) {
        Serial.println("Found saved WiFi credentials in EEPROM:");
        Serial.println("SSID: " + savedSSID);
        Serial.println("Password: " + savedPassword);
    }
    else
        Serial.println("No saved WiFi credentials found.");
}

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n \n \n");

    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);
    // clearEEPROM();

    // Initialize WIFI_SETUP_BUTTON
    pinMode(WIFI_SETUP_BUTTON, INPUT_PULLUP);
    lastButtonState = digitalRead(WIFI_SETUP_BUTTON);
    currentButtonState = lastButtonState;

    // Check if WiFi credentials are already stored
    readWifiFromEEPROM();
}

void loop() {
    server.handleClient();

    // Check Wifi Setup Button
    int reading = digitalRead(WIFI_SETUP_BUTTON);
    if (reading != lastButtonState) lastDebounceTime = millis();
    if ((millis() - lastDebounceTime) > debounceDelay && reading != currentButtonState) {
        currentButtonState = reading;
        if (currentButtonState == LOW) {
            setupWifi();
        }
    }
    lastButtonState = reading;
}
