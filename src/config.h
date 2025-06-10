#pragma once

#include <Arduino.h>
#include <vector>

struct Card {
    String id;
    String file;
    String name;
    bool has_photo;
};

struct Config {
    int default_volume;
    String audiodb_path;
    String unknown_card_sfx;
    std::vector<Card> cards;
};

extern const String CONF_PATH;