#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include "led_utils.h"
#include "html_pages.h"
#include "eeprom_utils.h"

// AP Configuration
const char* ap_ssid = "Mintek_Blinds";
IPAddress ap_ip(192, 168, 1, 1);       // Static IP address for AP
IPAddress ap_gateway(192, 168, 1, 1);  // Gateway IP (usually same as AP IP)
IPAddress ap_subnet(255, 255, 255, 0); // Subnet mask

// Web Server
ESP8266WebServer server(80);

// WiFi State Variables
bool wifiSetupActive = false;
bool credentialsSubmitted = false;  // Flag to track when credentials are submitted

// WiFi Station Static IP Configuration
IPAddress wifi_ip(192, 168, 68, 136);      // Static IP address for WiFi station
IPAddress wifi_gateway(192, 168, 68, 1);   // Gateway IP (usually your router IP)
IPAddress wifi_subnet(255, 255, 255, 0);  // Subnet mask

// Handle AP Configuration Page
void handleAPSetupPage(){
  String s = WIFI_SETUP;
  server.send(200,"text/html",s);
}

void handleWifiHomePage(){
  String s = WIFI_HOME;
  server.send(200,"text/html",s);
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
    setLedColor(255, 255, 255);
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
    server.on("/setup", handleAPSetupPage);
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
    setLedOff();
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
  setLedColor(0, 0, 255);
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

  Serial.print("Waiting: ");
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
    server.on("/", handleWifiHomePage);

    // Start the web server if not already started
    server.begin();
    Serial.println("HTTP server started on: http://" + WiFi.localIP().toString());
  }
  else
    Serial.println("Failed to connect to WiFi after " + String(attempt) + " attempts");

  setLedOff();
  Serial.println("----------------------------------------------------------------");
}

// Function to handle server clients (for use in loop)
void handleWiFiServer() {
    server.handleClient();
}

#endif // WIFI_UTILS_H

