#include "Audio.h"
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#define MAX_PATH_DEPTH 2

SPIClass *spi_onboardSD = new SPIClass(FSPI);
Audio audio;
File root, entry;
bool playing = false;
std::vector<File> dirChain;

String findNextAudioFile() {
  while (dirChain.size()) {
    entry = dirChain[dirChain.size() - 1].openNextFile();
    if (!entry) {
      dirChain[dirChain.size() - 1].close();
      dirChain.pop_back();
      continue;
    }

    if (entry.isDirectory()) {
      if (dirChain.size() < MAX_PATH_DEPTH) {
        dirChain.push_back(entry);  // dir entry stays open while member of chain
        continue;
      }
    }
    else if (String(entry.name()).endsWith("mp3")) {
      String filePath = entry.path();
      entry.close();
      return filePath;
    }
    entry.close();
  }
  return "";  // No more files found
}

void playNextTrack() {
  String nextFile = findNextAudioFile();
  if (nextFile != "") {
    audio.connecttoFS(SD, nextFile.c_str());
    playing = true;
  }
}

void audio_eof_mp3(const char *info) {
  Serial.print("eof_mp3: ");
  playing = false;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  while (!Serial);
  delay(3000);
  spi_onboardSD->begin();

  if (!SD.begin(SS, *spi_onboardSD)) {
    Serial.println("error mounting microSD");
    return;
  }
  digitalWrite(LED_BUILTIN, HIGH);

  root = SD.open("/");
  dirChain.push_back(root);

  audio.setPinout(I2S_BCLK, I2S_LRCLK, I2S_DOUT);
  audio.setVolume(6); // 0...21
}

void loop() {
  if (!playing) {
    playNextTrack();
  }
  audio.loop();
  vTaskDelay(1);
}