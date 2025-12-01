#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <EEPROM.h>
#include <Arduino.h>

// EEPROM Configuration
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASSWORD_ADDR 64
#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 64

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

#endif // EEPROM_UTILS_H

