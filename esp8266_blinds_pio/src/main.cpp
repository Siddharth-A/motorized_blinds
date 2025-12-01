/*
ESP8266 AP Mode - WiFi Configuration Portal
*/


#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "wifi-config.h"
#include "wifi-home.h"

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
bool credentialsSubmitted = false;  // Flag to track when credentials are submitted

// WiFi Station Static IP Configuration
IPAddress wifi_ip(192, 168, 68, 136);      // Static IP address for WiFi station
IPAddress wifi_gateway(192, 168, 68, 1);   // Gateway IP (usually your router IP)
IPAddress wifi_subnet(255, 255, 255, 0);  // Subnet mask


// Handle AP Configuration Page
void handleAPConfigurationPage(){
  String s = WIFI_CONFIG;
  server.send(200,"text/html",s);
}

void handleWifiHomePage(){
  String s = WIFI_HOME;
  server.send(200,"text/html",s);
}

void writeStringToEEPROM(int address, String data) {
    // Ensure we don't exceed max length
    int len = data.length();
    if (len > MAX_SSID_LEN && address == SSID_ADDR) {
        len = MAX_SSID_LEN;
        data = data.substring(0, MAX_SSID_LEN);
    }
    if (len > MAX_PASSWORD_LEN && address == PASSWORD_ADDR) {
        len = MAX_PASSWORD_LEN;
        data = data.substring(0, MAX_PASSWORD_LEN);
    }

    // Write length byte
    EEPROM.write(address, len);

    // Write string data (without null terminator, just raw bytes)
    for (int i = 0; i < len; i++) {
        EEPROM.write(address + 1 + i, data[i]);
    }

    // Clear any remaining bytes in the allocated space
    int maxLen = (address == SSID_ADDR) ? MAX_SSID_LEN : MAX_PASSWORD_LEN;
    for (int i = len; i < maxLen; i++) {
        EEPROM.write(address + 1 + i, 0);
    }
    EEPROM.commit();
}

String readStringFromEEPROM(int address) {
    // Read length byte
    uint8_t len = EEPROM.read(address);

    // Validate length - check against appropriate max
    int maxLen = (address == SSID_ADDR) ? MAX_SSID_LEN : MAX_PASSWORD_LEN;
    if (len == 0 || len > maxLen) {
        return "";
    }
    // Read string data
    String data = "";
    for (int i = 0; i < len; i++) {
        char c = char(EEPROM.read(address + 1 + i));
        // Stop if we encounter a null byte (safety check)
        if (c == '\0') {
            break;
        }
        data += c;
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

        // Set flag to indicate credentials have been submitted
        credentialsSubmitted = true;

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
    Serial.println("\n \n \n");
    Serial.println("----------------------------------------------------------------");
    Serial.println("Starting WiFi Configuration Portal...");

    // Reset flags to allow re-entry
    wifiSetupActive = false;
    credentialsSubmitted = false;

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
    Serial.println("Connect to this network and navigate to http://" + IP.toString() + "/setup");

    // Set up web server routes (only register once)
    server.on("/setup", handleAPConfigurationPage);
    server.on("/wifi-config", HTTP_POST, handleWiFiConfig);
    server.on("/clear-eeprom", HTTP_GET, handleClearEEPROM);  // Clear EEPROM via GET request
    server.onNotFound(handleNotFound);

    // Start server on submitting WiFi credentials
    server.begin();
    Serial.println("HTTP server started");

    wifiSetupActive = true;
    Serial.println("----------------------------------------------------------------");
    Serial.println("Waiting for WiFi credentials to be submitted...");

    // Wait for credentials to be submitted
    unsigned long startTime = millis();
    unsigned long timeout = 300000; // 5 minutes timeout (300000 ms)
    
    while (!credentialsSubmitted) {
        // Handle web server requests
        server.handleClient();
        
        // Check for timeout
        if (millis() - startTime > timeout) {
            Serial.println("Timeout: No credentials submitted within 5 minutes");
            break;
        }
        
        // Small delay to prevent tight loop
        delay(100);
    }
    
    if (credentialsSubmitted) {
        Serial.println("WiFi credentials have been submitted!");
    }
    Serial.println("----------------------------------------------------------------");
    
    // Reset wifiSetupActive to allow re-entry if needed
    wifiSetupActive = false;
}

bool readWifiCredentialsFromEEPROM() {
  Serial.println("\n \n \n");
  Serial.println("----------------------------------------------------------------");
  Serial.println("Checking for WiFi credentials in EEPROM...");
  bool result = false;
    String savedSSID = readStringFromEEPROM(SSID_ADDR);
    String savedPassword = readStringFromEEPROM(PASSWORD_ADDR);

    if (savedSSID.length() > 0) {
        Serial.println("Found saved WiFi credentials in EEPROM:");
        Serial.println("SSID: " + savedSSID);
        Serial.println("Password: " + savedPassword);
        result = true;
    }
    else
        Serial.println("No saved WiFi credentials found.");
    Serial.println("----------------------------------------------------------------");
    return result;
}

void connectToWiFi() {
  Serial.println("\n \n \n");
  Serial.println("----------------------------------------------------------------");
  Serial.println("Connecting to WiFi...");
  String savedSSID = readStringFromEEPROM(SSID_ADDR);
  String savedPassword = readStringFromEEPROM(PASSWORD_ADDR);

  // Turn off Access Point mode and connect to WiFi
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

  // Configure static IP address
  WiFi.config(wifi_ip, wifi_gateway, wifi_subnet);

  Serial.print("Connecting");
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 10) {
    Serial.print(".");
    delay(5000);
    attempt++;
  }
  Serial.println("\n");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi: " + savedSSID);
    Serial.println("IP address: " + WiFi.localIP().toString());
    Serial.println("Gateway IP: " + WiFi.gatewayIP().toString());
    server.on("/", handleWifiHomePage);
  }
  else
    Serial.println("Failed to connect to WiFi after " + String(attempt) + " attempts");
  Serial.println("----------------------------------------------------------------");
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
    if (readWifiCredentialsFromEEPROM())
      connectToWiFi();
    else{
      setupWifi();
      connectToWiFi();
    }
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
            connectToWiFi();
        }
    }
    lastButtonState = reading;
}
