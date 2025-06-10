#pragma once

#include <Arduino.h>
#include <MFRC522.h>

#define RST_PIN 2

class NFCReader {
private:
    MFRC522 mfrc522;
    
public:
    NFCReader();
    void initialize(SPIClass* spi);
    bool is_card_present();
    String get_card_uid();
    void halt_card();
};