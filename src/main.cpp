#include "app.h"
#include "config.h"
#include "debug.h"
#include "display_manager.h"
#include "hardware.h"
#include "input_handler.h"
#include "nfc_reader.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

const String CONF_PATH = "/config.yaml";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DisplayManager display_manager(&display);
App app(display_manager);
InputHandler input_handler(app);
NFCReader nfc_reader;

void handle_nfc() {
    if (!nfc_reader.is_card_present()) {
        return;
    }

    String card_uid = nfc_reader.get_card_uid();
    debug_print("NFC Card detected: %s", card_uid.c_str());

    app.play(card_uid);
    nfc_reader.halt_card();
    delay(1000);
}

void setup() {
    Serial.begin(115200);
    delay(3000);
    debug_print("Starting up...");

    if (!Hardware::initialize_sd_card()) {
        return;
    }

    Hardware::initialize_spi();
    nfc_reader.initialize(Hardware::spi_rc522);
    debug_print("RC522 initialized");

    if (!Hardware::initialize_display()) {
        return;
    }

    app.setup();

    pinMode(LED_BUILTIN, OUTPUT);
    debug_print("Ready!");
}

void loop() {
    input_handler.handle_keyboard_input();
    handle_nfc();
    app.loop();
    vTaskDelay(1);
}

void audio_info(const char *info) {
    debug_print("Audio info: %s", info);
}

void audio_eof_mp3(const char *info) {
    debug_print("Audio finished: %s", info);
    app.on_song_finished();
}