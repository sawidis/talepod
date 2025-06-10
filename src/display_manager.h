#pragma once

#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <vector>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

class DisplayManager {
private:
    Adafruit_SSD1306* oled;
    
public:
    DisplayManager(Adafruit_SSD1306* display);
    
    void display_rows(const std::vector<String>& rows, int text_size = 1);
    void show_playing(const String& title);
    void reset();
    void draw_centered_bitmap(const String& bmp_path);
};