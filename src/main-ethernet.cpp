#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("Hello, world!");
}

void loop() {
    Serial.println("Hello, world!");
    delay(1000);
}