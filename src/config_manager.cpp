#include "config_manager.h"
#include "debug.h"
#include <FS.h>
#include <SD.h>
#include <SPIFFS.h>
#include <YAMLDuino.h>

String ConfigManager::get_yaml_string(const YAMLNode& parent, const char* key, const String& default_value) {
    YAMLNode node = parent[key];
    return (!node.isNull() && node.isScalar()) ? String(node.scalar()) : default_value;
}

int ConfigManager::get_yaml_int(const YAMLNode& parent, const char* key, int default_value) {
    YAMLNode node = parent[key];
    return (!node.isNull() && node.isScalar()) ? atoi(node.scalar()) : default_value;
}

String ConfigManager::get_card_bmp_path(const Config& config, const Card& card) {
    String audio_path = config.audiodb_path + "/" + card.file;
    return audio_path + ".bmp";
}

std::optional<Config> ConfigManager::load_config(const String& conf_path) {
    File config_file;

    if (SD.begin() && SD.exists(conf_path)) {
        config_file = SD.open(conf_path);
        debug_print("Loading config from SD card");
    }
    else if (SPIFFS.begin() && SPIFFS.exists(conf_path)) {
        config_file = SPIFFS.open(conf_path);
        debug_print("Loading config from SPIFFS");
    } else {
        debug_print("Configuration file not found");
        return std::nullopt;
    }

    String yaml_content = "";
    while (config_file.available()) {
        yaml_content += (char)config_file.read();
    }
    config_file.close();

    debug_print("Parsing YAML configuration...");

    YAMLNode root;
    try {
        root = YAMLNode::loadString(yaml_content.c_str());
    } catch (const std::exception& e) {
        debug_print("YAML parse error: %s", e.what());
        return std::nullopt;
    } catch (const std::runtime_error& e) {
        debug_print("YAML runtime error: %s", e.what());
        return std::nullopt;
    } catch (...) {
        debug_print("Unknown YAML parsing error");
        return std::nullopt;
    }

    Config config;
    config.default_volume = get_yaml_int(root, "default_volume", 10);
    config.audiodb_path = get_yaml_string(root, "audiodb_path", "audiodb");
    config.unknown_card_sfx = get_yaml_string(root, "unknown_card_sfx", "default.mp3");

    YAMLNode cards_node = root["cards"];
    if (!cards_node.isNull() && cards_node.isSequence()) {
        for (size_t i = 0; i < cards_node.size(); i++) {
            YAMLNode card_node = cards_node[i];
            if (card_node.isMap()) {
                Card card;
                card.id = get_yaml_string(card_node, "id");
                card.file = get_yaml_string(card_node, "file");
                card.name = get_yaml_string(card_node, "name");
                card.has_photo = SD.exists(get_card_bmp_path(config, card));

                if (!card.id.isEmpty() && !card.file.isEmpty()) {
                    config.cards.push_back(card);
                }
            }
        }
    }

    debug_print("Configuration loaded successfully!");
    debug_print("Default Volume: %d", config.default_volume);
    debug_print("Audio DB Path: %s", config.audiodb_path.c_str());
    debug_print("Unknown Card SFX: %s", config.unknown_card_sfx.c_str());

    debug_print("Cards loaded: %d", config.cards.size());
    for (const auto& card : config.cards) {
        debug_print("  ID=%s, File=%s, Name=%s",
                      card.id.c_str(),
                      card.file.c_str(),
                      card.name.c_str());
    }

    return config;
}