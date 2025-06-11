#include "input_handler.h"
#include "debug.h"
#include <Arduino.h>

// Static instance for interrupt handling
InputHandler* InputHandler::instance = nullptr;

InputHandler::InputHandler(App& application) 
    : app(application), rotation_detected(false), clockwise(false), 
      last_button_state(HIGH), last_button_press_time(0), last_rotation_time(0) {
    instance = this;
}

void IRAM_ATTR InputHandler::rotary_interrupt() {
    if (instance) {
        unsigned long current_time = millis();
        if (current_time - instance->last_rotation_time < instance->ROTATION_DEBOUNCE) {
            return; // Too soon, ignore
        }
        
        // Read both pins
        bool clk_state = digitalRead(ENCODER_CLK_PIN);
        bool dt_state = digitalRead(ENCODER_DT_PIN);
        
        // Only process on falling edge of CLK (one complete step)
        if (!clk_state) {
            instance->clockwise = (clk_state != dt_state);
            instance->rotation_detected = true;
            instance->last_rotation_time = current_time;
        }
    }
}

void InputHandler::initialize() {
    pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
    pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
    pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
    
    last_button_state = digitalRead(ENCODER_SW_PIN);
    
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), rotary_interrupt, FALLING);
    
    debug_print("Rotary encoder initialized with interrupt");
}

void InputHandler::handle_keyboard_input() {
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

void InputHandler::handle_rotary_encoder() {
    if (rotation_detected) {
        rotation_detected = false;
        
        if (clockwise) {
            debug_print("Rotary encoder: clockwise - increasing volume");
            app.incr_volume();
        } else {
            debug_print("Rotary encoder: counterclockwise - decreasing volume");
            app.decr_volume();
        }
    }
    
    // Handle button press (with debouncing)
    bool current_button_state = digitalRead(ENCODER_SW_PIN);
    unsigned long current_time = millis();
    
    // Check for button press (transition from HIGH to LOW)
    if (last_button_state == HIGH && current_button_state == LOW) {
        // Check if enough time has passed since last press (debouncing)
        if (current_time - last_button_press_time > DEBOUNCE_DELAY) {
            debug_print("Rotary encoder button pressed - toggle play/pause");
            app.toggle_play_pause();
            last_button_press_time = current_time;
        }
    }
    
    last_button_state = current_button_state;
}