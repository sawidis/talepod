#pragma once

#include "config.h"
#include "display_manager.h"
#include <Audio.h>
#include <optional>

enum AppState {
    APP_STATE_IDLE,
    APP_STATE_PLAYING,
    APP_STATE_PAUSED,
};

class App {
private:
    static const int MIN_VOLUME = 0;
    static const int MAX_VOLUME = 21;

    std::optional<Config> config;
    AppState state;
    Audio audio;
    std::optional<Card> active_card;
    int volume_level;
    DisplayManager display_manager;

    bool is_playing() const;
    bool is_paused() const;
    bool is_idle() const;
    void set_state(AppState new_state);
    void set_volume(int val);
    std::optional<Card> find_card_by_uid(const String& uid);
    void play_card(const std::optional<Card>& card);

public:
    App(DisplayManager& display_mgr);
    
    void setup();
    void loop();
    void play(const String& card_uid);
    void toggle_play_pause();
    void incr_volume();
    void decr_volume();
    void stop();
    void show_info();
    void on_song_finished();
};