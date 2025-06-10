#include "hardware.h"
#include "debug.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <SD.h>

SPIClass* Hardware::spi_rc522 = new SPIClass(HSPI);
SPIClass* Hardware::spi_onboard_sd = new SPIClass(FSPI);

extern Adafruit_SSD1306 display;

bool Hardware::initialize_sd_card() {
    spi_onboard_sd->begin();
    if (!SD.begin(SS, *spi_onboard_sd)) {
        debug_print("error mounting microSD");
        return false;
    }
    debug_print("SD card initialized");
    return true;
}

bool Hardware::initialize_display() {
    bool display_found = false;
    for (int addr : {0x3C, 0x3D}) {
        Serial.print("Trying address 0x");
        Serial.println(addr, HEX);

        if (display.begin(SSD1306_SWITCHCAPVCC, addr)) {
            Serial.println("Display initialized successfully!");
            display_found = true;
            break;
        }
    }
    
    if (!display_found) {
        Serial.println("Could not initialize display with any address");
        return false;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("Tailpod 3000"));
    display.display();
    
    return true;
}

void Hardware::initialize_spi() {
    spi_rc522->begin(SCK2, MISO2, MOSI2, SS2);
}

void Hardware::ping() {
    for (int i = 0; i <= 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}