#pragma once

#include <Arduino.h>
#include <MFRC522.h>

#define RST_PIN 2

class NFCReader {
private:
    MFRC522 mfrc522;

    // Presence tracking, so a card sitting on (or being lifted off) the reader
    // is not mistaken for a fresh tap.
    bool card_present;
    byte absence_count;
    unsigned long last_presence_check;

    // A halted card stops answering REQA, so its continued presence is probed
    // with WUPA. Throttle that probe and debounce removal against flaky reads
    // at the edge of the RF field.
    static const unsigned long PRESENCE_CHECK_INTERVAL = 200; // ms
    static const byte ABSENCE_THRESHOLD = 3;                  // consecutive misses

    bool is_card_still_present();

public:
    NFCReader();
    void initialize(SPIClass* spi);
    bool is_card_present();
    String get_card_uid();
    void halt_card();

    // Returns the UID of a newly presented card, or "" when nothing new happened
    // (no card, or the same card is still on / leaving the reader).
    String poll_new_card();
};
