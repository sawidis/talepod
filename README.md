# Talepod

An NFC-triggered audio player built with an ESP32-S3 and RC522 NFC reader.
Touch an NFC card to play associated MP3 files stored on an SD card.

Made with love to make little loved ones happy :heart:


## Features

- **NFC-triggered playback** - Touch a card to play its associated audio file
- **OLED Display** - Shows current status and bitmap images for cards
- **Keyboard controls** - Play/pause, volume controls, and stop via serial interface
- **SD card storage** - All audio files stored locally on SD card for device portability
- **YAML configuration** - Easy configuration for associating cards with audio files
- **Default fallback** - Plays default audio for unknown cards
- **Media controls** - Volume up/down, pause/resume, stop
- **Bitmap support** - Displays card-specific images on OLED when available

## Hardware Requirements

- **YB-ESP32-S3-AMP** development board
- **RC522 NFC module**
- **SSD1306 OLED display** (128x64 pixels, I2C)
- **Micro SD card** (formatted as FAT32)
- **Speakers** (3W, connected to ESP32-S3-AMP I2S audio output)
- **NFC cards/tags** (MIFARE Classic or compatible)
- **Rotary encoder**


## Wiring

### RC522 NFC Module
| RC522 Pin | ESP32-S3 Pin |
|-----------|--------------|
| VCC       | 3.3V         |
| GND       | GND          |
| MISO      | GPIO 41      |
| MOSI      | GPIO 39      |
| SCK       | GPIO 40      |
| SDA/CS    | GPIO 38      |
| RST       | GPIO  2      |

### Display Module
| Display Pin | ESP32-S3 Pin |
|-------------|--------------|
| VCC         | 3.3V         |
| GND         | GND          |
| SCK         | GPIO  9      |
| SDA/CS      | GPIO  8      |

### Rotary Encoder (volume control + pause/resume)
| Encoder Pin | ESP32-S3 Pin |
|-------------|--------------|
| GND         | GND          |
| +           | 3.3V         |
| SW          | GPIO 15      |
| DT          | GPIO 16      |
| CLK         | GPIO 17      |


## SD Card Layout

```
/sdcard/
└── audiodb/                    # configured via `audiodb_path`
    └── three_little_pigs.mp3
    └── three_little_pigs.mp3.bmp
    └── ministry_for_the_future.mp3
    └── sad_trombone.mp3
    └── sad_trombone.mp3.bmp
```

When an audio file `x.mp3` is triggered, Talepod will search for a file `x.mp3.bmp`
at the same dir as `x.mp3`. If one exists, it will try to render it on the display
while `x.mp3` is playing.

To generate a compatible bmp:

```
# assuming the generated bmp will be a companion to sad_trombone.mp3
convert sadface.jpg -resize 48x48 -monochrome sad_trombone.mp3.bmp
```


## Configuration

Edit `data/config.yaml` taking into account the audio files in the SD card:

```yaml
default_volume: 5
audiodb_path: "audiodb"
unknown_card_sfx: "sad_trombone.mp3"
cards:
  - id: "E5:F6:G7:H8"
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
