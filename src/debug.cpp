#include "debug.h"
#include <Arduino.h>
#include <stdarg.h>

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