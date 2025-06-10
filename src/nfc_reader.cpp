#include "nfc_reader.h"

NFCReader::NFCReader() : mfrc522(SS2, RST_PIN) {}

void NFCReader::initialize(SPIClass* spi) {
    spi->begin(SCK2, MISO2, MOSI2, SS2);
    SPI = *spi;
    mfrc522.PCD_Init(SS2, RST_PIN);
}

bool NFCReader::is_card_present() {
    return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}

String NFCReader::get_card_uid() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (uid != "") uid += ":";
        if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    return uid;
}

void NFCReader::halt_card() {
    mfrc522.PICC_HaltA();
}