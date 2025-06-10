#include "display_manager.h"
#include "debug.h"
#include <SD.h>

DisplayManager::DisplayManager(Adafruit_SSD1306* display) : oled(display) {}

void DisplayManager::display_rows(const std::vector<String>& rows, int text_size) {
    oled->clearDisplay();
    oled->setCursor(0, 0);
    oled->setTextSize(text_size);
    
    for (const auto& row : rows) {
        oled->println(row);
    }
    
    oled->display();
}

void DisplayManager::show_playing(const String& title) {
    display_rows({"Now playing..", title});
}

void DisplayManager::reset() {
    display_rows({"Tailpod 3000"});
}

void DisplayManager::draw_centered_bitmap(const String& bmp_path) {
    oled->clearDisplay();
    
    if (!SD.exists(bmp_path)) {
        debug_print("File not found: %s", bmp_path.c_str());
        return;
    }
    
    File bmp_file = SD.open(bmp_path);
    if (!bmp_file) {
        debug_print("Failed to open file: %s", bmp_path.c_str());
        return;
    }
    
    uint16_t signature = bmp_file.read() | (bmp_file.read() << 8);
    if (signature != 0x4D42) {
        debug_print("Invalid BMP signature: 0x%X", signature);
        bmp_file.close();
        return;
    }
    
    bmp_file.seek(10);
    uint32_t data_offset = bmp_file.read() | (bmp_file.read() << 8) | (bmp_file.read() << 16) | (bmp_file.read() << 24);
    
    bmp_file.seek(18);
    int32_t width = bmp_file.read() | (bmp_file.read() << 8) | (bmp_file.read() << 16) | (bmp_file.read() << 24);
    int32_t height = bmp_file.read() | (bmp_file.read() << 8) | (bmp_file.read() << 16) | (bmp_file.read() << 24);
    
    bmp_file.seek(28);
    uint16_t bits_per_pixel = bmp_file.read() | (bmp_file.read() << 8);
    
    debug_print("BMP: %dx%d, %d-bit, data offset: %d", width, height, bits_per_pixel, data_offset);
    
    if (width <= 0 || height <= 0 || width > SCREEN_WIDTH || height > SCREEN_HEIGHT) {
        debug_print("Invalid dimensions: %dx%d (max: %dx%d)", width, height, SCREEN_WIDTH, SCREEN_HEIGHT);
        bmp_file.close();
        return;
    }
    
    if (bits_per_pixel != 1) {
        debug_print("Only 1-bit BMPs supported, got %d-bit", bits_per_pixel);
        bmp_file.close();
        return;
    }
    
    int x = (SCREEN_WIDTH - width) / 2;
    int y = (SCREEN_HEIGHT - height) / 2;
    
    debug_print("Drawing at position: (%d,%d)", x, y);
    
    bmp_file.seek(data_offset);
    
    int bytes_per_row = (width + 7) / 8;
    int padded_row_size = ((width + 31) / 32) * 4;
    int padding_bytes = padded_row_size - bytes_per_row;
    
    debug_print("Row info: %d bytes per row, %d padding bytes", bytes_per_row, padding_bytes);
    
    for (int row = height - 1; row >= 0; row--) {
        for (int col = 0; col < width; col += 8) {
            uint8_t pixel_byte = bmp_file.read();
            
            for (int bit = 7; bit >= 0 && (col + (7-bit)) < width; bit--) {
                if (!(pixel_byte & (1 << bit))) {
                    oled->drawPixel(x + col + (7-bit), y + row, SSD1306_WHITE);
                }
            }
        }
        
        for (int i = 0; i < padding_bytes; i++) {
            bmp_file.read();
        }
    }
    
    bmp_file.close();
    oled->display();
    
    debug_print("Bitmap drawn successfully");
}