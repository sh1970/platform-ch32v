#include <Arduino.h>

// Depending on build flags, Serial should either be regular UART or USB CDC!
void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Hello, world!");
  delay(1000);
}