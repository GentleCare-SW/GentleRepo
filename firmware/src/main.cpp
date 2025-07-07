#include <Arduino.h>

void setup()
{
    Serial.begin(BAUD_RATE);
}

void loop()
{
    Serial.println("Hello, World!");
    delay(1000);
}