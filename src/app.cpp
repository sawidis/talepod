#include "app.h"
#include "config_manager.h"
#include "debug.h"
#include <SD.h>

App::App(DisplayManager& display_mgr) 
    : state(APP_STATE_IDLE), volume_level(0), display_manager(display_mgr) {}

bool App::is_playing() const { 
    return state == APP_STATE_PLAYING; 
}

bool App::is_paused() const { 
    return state == APP_STATE_PAUSED; 
}

bool App::is_idle() const { 
    return state == APP_STATE_IDLE; 
}

void App::set_state(AppState new_state) { 
    state = new_state; 
}

void App::set_volume(int val) {
    volume_level = val;
    audio.setVolume(volume_level);
}

std::optional<Card> App::find_card_by_uid(const String& uid) {
    for (const auto& card : config.value().cards) {
        if (card.id == uid) {
            return card;
        }
    }
    return std::nullopt;
}

void App::play_card(const std::optional<Card>& card) {
    String audio_db_path = config.value().audiodb_path;

    if (!card.has_value()) {
        String fallback = audio_db_path + "/" + config.value().unknown_card_sfx;
        display_manager.draw_centered_bitmap(fallback + ".bmp");
        audio.connecttoFS(SD, fallback.c_str());
        return;
    }

    String path = audio_db_path + "/" + card.value().file;
    if (!SD.exists(path)) {
        debug_print("Audio file not found: %s", path.c_str());
        String fallback = audio_db_path + "/" + config.value().unknown_card_sfx;
        audio.connecttoFS(SD, fallback.c_str());
        return;
    }

    if (is_playing()) {
        audio.stopSong();
    }

    if (audio.connecttoFS(SD, path.c_str())) {
        active_card = card;
        if (active_card.value().has_photo) {
            String active_card_bmp_path = ConfigManager::get_card_bmp_path(config.value(), card.value());
            display_manager.draw_centered_bitmap(active_card_bmp_path);
        }
        set_state(APP_STATE_PLAYING);
        debug_print("Audio started successfully");
    } else {
        set_state(APP_STATE_IDLE);
        active_card.reset();
        display_manager.reset();
        debug_print("Failed to start audio");
    }
}

void App::setup() {
    config = ConfigManager::load_config(CONF_PATH);

    if (!config) {
        debug_print("Failed to load config!");
        return;
    }
    debug_print("Config loaded successfully");

    audio.setPinout(I2S_BCLK, I2S_LRCLK, I2S_DOUT);
    set_volume(config.value().default_volume);
}

void App::loop() {
    audio.loop();
}

void App::play(const String& card_uid) {
    std::optional<Card> card = find_card_by_uid(card_uid);
    if (!card.has_value()) {
        debug_print("No audio entry found with id %s", card_uid.c_str());
    }
    play_card(card);
}

void App::toggle_play_pause() {
    if (is_paused()) {
        audio.pauseResume();
        set_state(APP_STATE_PLAYING);
        debug_print("Audio resumed");
    } else if (is_playing()) {
        audio.pauseResume();
        set_state(APP_STATE_PAUSED);
        debug_print("Audio paused");
    } else {
        if (active_card.has_value()) {
            play_card(active_card.value());
        }
    }
}

void App::incr_volume() {
    if (volume_level == MAX_VOLUME) {
        return;
    }
    set_volume(volume_level + 1);
}

void App::decr_volume() {
    if (volume_level == MIN_VOLUME) {
        return;
    }
    set_volume(volume_level - 1);
}

void App::stop() {
    if (!is_playing()) {
        debug_print("No audio is currently playing");
        return;
    }
    audio.stopSong();
    set_state(APP_STATE_IDLE);
    display_manager.reset();
    debug_print("Audio stopped");
}

void App::show_info() {
    debug_print("=== Current Status ===");
    debug_print("Current volume: %d", volume_level);
    debug_print("Current state: %d", (int)state);

    if (auto active_card_ref = active_card; active_card_ref.has_value()) {
        debug_print("Active card: %s (%s)", 
                   active_card_ref.value().name.c_str(), 
                   active_card_ref.value().id.c_str());
    } else {
        debug_print("No active card");
    }
}

void App::on_song_finished() {
    set_state(APP_STATE_IDLE);
    active_card.reset();
    display_manager.reset();
    debug_print("Song finished - state set to idle");
}