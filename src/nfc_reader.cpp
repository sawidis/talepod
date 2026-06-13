#include "nfc_reader.h"

NFCReader::NFCReader()
    : mfrc522(SS2, RST_PIN), card_present(false), absence_count(0),
      last_presence_check(0) {}

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

// A halted card ignores REQA, so we wake it with WUPA to confirm it is still
// physically on the reader. PICC_Select() (run while reading the UID) leaves the
// data-rate registers in a state where WUPA fails, so reset them first - the same
// preamble the library uses in PICC_IsNewCardPresent().
bool NFCReader::is_card_still_present() {
    mfrc522.PCD_WriteRegister(MFRC522::TxModeReg, 0x00);
    mfrc522.PCD_WriteRegister(MFRC522::RxModeReg, 0x00);
    mfrc522.PCD_WriteRegister(MFRC522::ModWidthReg, 0x26);

    byte atqa[2];
    byte size = sizeof(atqa);
    MFRC522::StatusCode status = mfrc522.PICC_WakeupA(atqa, &size);
    if (status == MFRC522::STATUS_OK || status == MFRC522::STATUS_COLLISION) {
        mfrc522.PICC_HaltA(); // put it back to sleep until the next probe
        return true;
    }
    return false;
}

String NFCReader::poll_new_card() {
    if (card_present) {
        unsigned long now = millis();
        if (now - last_presence_check < PRESENCE_CHECK_INTERVAL) {
            return ""; // not time to re-probe; assume still present
        }
        last_presence_check = now;

        if (is_card_still_present()) {
            absence_count = 0;
            return "";
        }
        // Tolerate a few misses while the card is being lifted through the
        // weak edge of the field before declaring it removed.
        if (++absence_count < ABSENCE_THRESHOLD) {
            return "";
        }
        card_present = false;
        absence_count = 0;
        return "";
    }

    if (is_card_present()) {
        String uid = get_card_uid();
        halt_card(); // silence the card so it won't re-trigger while it sits
        card_present = true;
        absence_count = 0;
        last_presence_check = millis();
        return uid;
    }
    return "";
}
