#include "input_handler.h"
#include "debug.h"
#include <Arduino.h>

InputHandler::InputHandler(App& application) : app(application) {}

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