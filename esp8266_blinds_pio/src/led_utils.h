#ifndef LED_UTILS_H
#define LED_UTILS_H

#include <FastLED.h>

#define NEOPIXEL_LED 14
#define NEOPIXEL_COUNT 1
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Global LED array
extern CRGB leds[NEOPIXEL_COUNT];

void printSeparator(int case_num) {
    switch (case_num) {
        case 1:
            Serial.println("\n \n \n");
            Serial.println("----------------------------------------------------------------");
            break;
        case 2:
            Serial.println("----------------------------------------------------------------");
            Serial.println("\n \n \n");
            break;
        default:
            Serial.println("----------------------------------------------------------------");
            break;
    }
}

// Initialize the LED
void initLed() {
    FastLED.addLeds<LED_TYPE, NEOPIXEL_LED, COLOR_ORDER>(leds, NEOPIXEL_COUNT);
    FastLED.setBrightness(100);
    FastLED.clear();
    FastLED.show();
}

// Set LED to a specific RGB color
void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
    leds[0] = CRGB(r, g, b);
    FastLED.show();
}

void setLedOff() {
    leds[0] = CRGB::Black;
    delay(1000);
    FastLED.show();
    leds[0] = CRGB::Black;
    delay(1000);
    FastLED.show();
}

#endif // LED_UTILS_H
