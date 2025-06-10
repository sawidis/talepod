#pragma once

#include "app.h"

class InputHandler {
private:
    App& app;
    
public:
    InputHandler(App& application);
    void handle_keyboard_input();
};