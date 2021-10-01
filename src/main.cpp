#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <arduino-timer.h>

Timer<10> timer;

SoftwareSerial bluetooth(2, 3); // RX, TX
void setup()
{
    pinMode(9, OUTPUT); // key pin
    digitalWrite(9, HIGH);

    Serial.begin(9600);
    bluetooth.begin(9600);

    Serial.println("Starting serial comms..!");

    // Led
    pinMode(6, OUTPUT);
}

bool activateMotor (void *) {
    digitalWrite(6, HIGH);
    return false;
}

bool deactivateMotor (void *) {
    digitalWrite(6, LOW);
    return false;
}

// Command example:  CAR+RUN:15
void handleBluetoothCommands(String command)
{
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, command);
    const char* commandType = doc["command"];

    Serial.println(commandType);

    if (strcmp(commandType, "RUN") == 0) {
        Serial.println("running command activated");
        activateMotor(NULL);
        const int time = (doc["time"] ? doc["time"] : 10)*1000;
        Serial.println(time);
        timer.in(time, deactivateMotor);
    }
}

void loop()
{
    if (bluetooth.available())
    {
        String response = bluetooth.readString();
        Serial.println(response);
        handleBluetoothCommands(response);
    }

    timer.tick();
}