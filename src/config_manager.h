#pragma once

#include "config.h"
#include <optional>
#include <YAMLDuino.h>

class ConfigManager {
public:
    static std::optional<Config> load_config(const String& conf_path);
    static String get_card_bmp_path(const Config& config, const Card& card);

private:
    static String get_yaml_string(const class YAMLNode& parent, const char* key, const String& default_value = "");
    static int get_yaml_int(const class YAMLNode& parent, const char* key, int default_value = 0);
};