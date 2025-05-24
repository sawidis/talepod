#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>
#include <FS.h>
#include <optional>
#include <YAMLDuino.h>
#include "Audio.h"
#include <LittleFS.h>

#define RST_PIN     2

const String CONF_PATH = "/config.yaml";

struct Card {
  String id;
  String file;
  String name;
};

struct Config {
  int defaultVolume;
  String audiodbPath;
  String unknownCardSfx;
  std::vector<Card> cards;
};

enum AppState {
  APP_STATE_IDLE,           // No audio loaded/stopped
  APP_STATE_PLAYING,        // Audio is currently playing
  APP_STATE_PAUSED,         // Audio is paused
};

bool debug = true;

void debug_print(const char* format, ...) {
  if (!debug) {
    return;
  }
  va_list args;
  va_start(args, format);

  Serial.print("[DEBUG] ");
  Serial.vprintf(format, args);
  Serial.println();

  va_end(args);
}

String get_yaml_string(const YAMLNode& parent, const char* key, const String& default_value = "") {
  YAMLNode node = parent[key];
  return (!node.isNull() && node.isScalar()) ? String(node.scalar()) : default_value;
}

int get_yaml_int(const YAMLNode& parent, const char* key, int default_value = 0) {
  YAMLNode node = parent[key];
  return (!node.isNull() && node.isScalar()) ? atoi(node.scalar()) : default_value;
}

std::optional<Config> load_config(String conf_path) {
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
  config.defaultVolume = get_yaml_int(root, "default_volume", 10);
  config.audiodbPath = get_yaml_string(root, "audiodb_path", "audiodb");
  config.unknownCardSfx = get_yaml_string(root, "unknown_card_sfx", "default.mp3");

  YAMLNode cardsNode = root["cards"];
  if (!cardsNode.isNull() && cardsNode.isSequence()) {
    for (size_t i = 0; i < cardsNode.size(); i++) {
      YAMLNode cardNode = cardsNode[i];
      if (cardNode.isMap()) {
        Card card;
        card.id = get_yaml_string(cardNode, "id");
        card.file = get_yaml_string(cardNode, "file");
        card.name = get_yaml_string(cardNode, "name");

        if (!card.id.isEmpty() && !card.file.isEmpty()) {
          config.cards.push_back(card);
        }
      }
    }
  }

  debug_print("Configuration loaded successfully!");
  debug_print("Default Volume: %d", config.defaultVolume);
  debug_print("Audio DB Path: %s", config.audiodbPath.c_str());
  debug_print("Unknown Card SFX: %s", config.unknownCardSfx.c_str());

  debug_print("Cards loaded: %d", config.cards.size());
  for (const auto& card : config.cards) {
    debug_print("  ID=%s, File=%s, Name=%s",
                  card.id.c_str(),
                  card.file.c_str(),
                  card.name.c_str());
  }

  return config;
}

class App {
  private:
    static const int MIN_VOLUME = 0;
    static const int MAX_VOLUME = 21;

    std::optional<Config> config;
    AppState state;
    Audio audio;
    std::optional<Card> active_card;
    int volume_level;

    bool is_playing() const { return state == APP_STATE_PLAYING; }
    bool is_paused() const { return state == APP_STATE_PAUSED; }
    bool is_idle() const { return state == APP_STATE_IDLE; }
    void set_state(AppState newState) { state = newState; }
    void set_volume(int val) {
      volume_level = val;
      audio.setVolume(volume_level);
    }

    std::optional<Card> find_card_by_uid(String uid) {
      for (const auto& card : config.value().cards) {
        if (card.id == uid) {
          return card;
        }
      }
      return std::nullopt;
    }

    void play_card(std::optional<Card> card) {
      String audio_db_path = config.value().audiodbPath;

      if (!card.has_value()) {
          String fallback = audio_db_path + "/" + config.value().unknownCardSfx;
          audio.connecttoFS(SD, fallback.c_str());
          return;
      }

      String path = audio_db_path + "/" + card.value().file;
      if (!SD.exists(path)) {
        debug_print("Audio file not found: %s", path.c_str());
        String fallback = audio_db_path + "/" + config.value().unknownCardSfx;
        audio.connecttoFS(SD, fallback.c_str());
        return;
      }

      if (is_playing()) {
        audio.stopSong();
      }

      if (audio.connecttoFS(SD, path.c_str())) {
        active_card = card;
        set_state(APP_STATE_PLAYING);
        debug_print("Audio started successfully");
      } else {
        set_state(APP_STATE_IDLE);
        active_card.reset();
        debug_print("Failed to start audio");
      }
    }

  public:
    App() : state(APP_STATE_IDLE), volume_level(0) {}

    void setup() {
      config = load_config(CONF_PATH);

      if (!config) {
        debug_print("Failed to load config!");
        return;
      }
      debug_print("Config loaded successfully");

      audio.setPinout(I2S_BCLK, I2S_LRCLK, I2S_DOUT);
      set_volume(config.value().defaultVolume);
    }

    void loop() {
      audio.loop();
    }

    void play(String card_uid) {
      std::optional<Card> card = find_card_by_uid(card_uid);
      if (!card.has_value()) {
        debug_print("No audio entry found with id %s", card_uid.c_str());
      }
      play_card(card);
    }

    void toggle_play_pause() {
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

    void incr_volume() {
      if (volume_level == MAX_VOLUME) {
        return;
      }
      set_volume(volume_level+1);
    }

    void decr_volume() {
      if (volume_level == MIN_VOLUME) {
        return;
      }
      set_volume(volume_level-1);
    }

    void stop() {
      if (!is_playing()) {
        debug_print("No audio is currently playing");
        return;
      }
      audio.stopSong();
      set_state(APP_STATE_IDLE);
      debug_print("Audio stopped");
    }

    void show_info() {
      debug_print("=== Current Status ===");
        debug_print("Current volume: %d", volume_level);
        debug_print("Current state: %d", (int)state);

        if (auto activeCard = active_card; activeCard.has_value()) {
          debug_print("Active card: %s (%s)", activeCard.value().name.c_str(), activeCard.value().id.c_str());
        } else {
          debug_print("No active card");
        }
      }
};

App app;

void handle_kb_input() {
  if (!Serial.available()) {
    return;
  }

  char key = Serial.read();

  switch (key) {
    case 'p':
      debug_print("pause/resume");
      app.toggle_play_pause();
      break;
    case '+':
      debug_print("Incr");
      app.incr_volume();
      break;
    case '-':
      debug_print("descr");
      app.decr_volume();
      break;
    case 's':
      debug_print("stop");
      app.stop();
      break;
    case 'i':
      debug_print("info");
      app.show_info();
      break;
    default:
      break;
  }
}

MFRC522 mfrc522(SS2, RST_PIN);


String get_card_uid() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (uid != "") uid += ":";
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void ping() {
  for (int i = 0; i <= 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void handle_nfc() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String card_uid = get_card_uid();
  debug_print("NFC Card detected: %s", card_uid.c_str());

  app.play(card_uid);

  // Halt communication with card
  mfrc522.PICC_HaltA();

  // Small delay to prevent rapid re-detection
  delay(1000);
}

SPIClass *spi_rc522 = new SPIClass(HSPI);
SPIClass *spi_onboardSD = new SPIClass(FSPI);

void setup() {
  Serial.begin(115200);
  delay(3000);
  debug_print("Starting up...");

  spi_onboardSD->begin();
  if (!SD.begin(SS, *spi_onboardSD)) {
    debug_print("error mounting microSD");
    return;
  }
  debug_print("SD card initialized");

  spi_rc522->begin(SCK2, MISO2, MOSI2, SS2);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);
  delay(100);
  SPI = *spi_rc522;
  mfrc522.PCD_Init(SS2, RST_PIN);
  debug_print("RC522 initialized");

  app.setup();

  pinMode(LED_BUILTIN, OUTPUT);
  debug_print("Ready!");
}

void loop() {
  handle_kb_input();
  handle_nfc();
  app.loop();
  vTaskDelay(1); // needed !
}

void audio_info(const char *info) {
  debug_print("Audio info: %s", info);
}

void audio_eof_mp3(const char *info) {
  debug_print("Audio finished: %s", info);
}