#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial bluetooth(2, 3); // RX, TX

void setup()
{
    Serial.begin(9600);
    while (!Serial);
    Serial.println("Starting serial communication...");
    bluetooth.begin(9600);
}

void loop()
{
    if (bluetooth.available()) {
        String response = bluetooth.readString();
        Serial.println(response);
    }
}