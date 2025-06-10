#pragma once

#include <SPI.h>

#define OLED_RESET -1
#define OLED_SDA_PIN 8
#define OLED_SCL_PIN 9

class Hardware {
public:
    static SPIClass* spi_rc522;
    static SPIClass* spi_onboard_sd;
    
    static bool initialize_sd_card();
    static bool initialize_display();
    static void initialize_spi();
    static void ping();
};