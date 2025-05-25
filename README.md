 # Talepod

An NFC-triggered audio player built with ESP32-S3 and RC522 NFC reader.
Touch an NFC card to play associated MP3 files stored on an SD card.

Made with love to make little loved ones happy :heart:

## Features

- **NFC-triggered playback** - Touch a card to play its associated audio file
- **Physical controls** - Play/Pause and Volume up/down buttons
- **SD card storage** - All audio files stored locally on SD card for device portability
- **Configurable mappings** - YAML configuration for associating cards with audio files
- **Default fallback** - Plays default audio for unknown cards

## Hardware Requirements

- **YB-ESP32-S3-AMP** development board
- **RC522 NFC module**
- **Micro SD card** (formatted as FAT32)
- **3 push buttons** (play/pause, volume up, volume down)
- **Speakers** (connected to ESP32-S3-AMP audio output)
- **NFC cards/tags** (MIFARE Classic or compatible)

## Wiring

### RC522 NFC Module
| RC522 Pin | ESP32-S3 Pin |
|-----------|--------------|
| VCC       | 3.3V         |
| GND       | GND          |
| MISO      | GPIO 12      |
| MOSI      | GPIO 13      |
| SCK       | GPIO 14      |
| SDA/CS    | GPIO 15      |
| RST       | GPIO 16      |

### Control Buttons
| Button      | ESP32-S3 Pin | Connection |
|-------------|--------------|------------|
| Play/Pause  | GPIO 21      | Button → GPIO 21, other terminal → GND |
| Volume Up   | GPIO 22      | Button → GPIO 22, other terminal → GND |
| Volume Down | GPIO 23      | Button → GPIO 23, other terminal → GND |

*Note: Internal pull-up resistors are enabled in software*

## Audio Files
```
/sdcard/
└── audiodb/                    # configured via `audiodb_path`
    └── three_little_pigs.mp3
    └── sad_trombone.mp3
```

## Configuration

Edit `data/config.yaml` taking into account the audio files in the SD card:

```yaml
default_volume: 5
audiodb_path: "audiodb"
unknown_card_sfx: "sad_trombone.mp3"
cards:
  - id: "E5F6G7H8"
    name: "Three Little Pigs"
    file: "three_little_pigs.mp3"
```

Then upload it to the board:

```
pio run --target uploadfs
```

## References & Inspiration

- [YB-ESP32-S3-AMP Getting Started Guide](https://github.com/yellobyte/ESP32-DevBoards-Getting-Started/tree/main/boards/YB-ESP32-S3-AMP)
- [How I Built an NFC Movie Library for My Kids](https://simplyexplained.com/blog/how-i-built-an-nfc-movie-library-for-my-kids/)