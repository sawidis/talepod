#pragma once

#include "app.h"

// Rotary encoder pins
#define ENCODER_SW_PIN 15
#define ENCODER_DT_PIN 16
#define ENCODER_CLK_PIN 17

class InputHandler {
private:
    App& app;
    
    // Rotary encoder state tracking
    volatile bool rotation_detected;
    volatile bool clockwise;
    bool last_button_state;
    unsigned long last_button_press_time;
    unsigned long last_rotation_time;
    static const unsigned long DEBOUNCE_DELAY = 50; // ms
    static const unsigned long ROTATION_DEBOUNCE = 5; // ms
    
    // Static instance for interrupt handling
    static InputHandler* instance;
    static void IRAM_ATTR rotary_interrupt();
    
public:
    InputHandler(App& application);
    void initialize();
    void handle_keyboard_input();
    void handle_rotary_encoder();
};