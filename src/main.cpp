#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <arduino-timer.h>
#include <vector>

Timer<10> timer;
std::vector<long> RunCommandBuffer;

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

void activateMotor () {
    digitalWrite(6, HIGH);
}

void deactivateMotor () {
    digitalWrite(6, LOW);
}

bool continueToRunIfMoreCommand(void *);
void runCar (long time) {
    char tbs[60];
    sprintf(tbs, "> Starting Run for time: %lu ms", time);
    Serial.println(tbs);

    activateMotor();
    timer.in(time, continueToRunIfMoreCommand);
}

bool continueToRunIfMoreCommand(void *) {
    Serial.println("> Finished a run");
    RunCommandBuffer.pop_back();
    if (RunCommandBuffer.size() > 0) {
        long time = RunCommandBuffer.at(RunCommandBuffer.size() - 1);
        runCar(time);
    }
    else deactivateMotor();

    return false;
}

// Command example:  CAR+RUN:15
void handleBluetoothCommands(String command)
{
    StaticJsonDocument<256> doc;
    deserializeJson(doc, command);
    const char* commandType = doc["command"];

    if (strcmp(commandType, "RUN") == 0) 
    {
        long docTime = doc["time"];
        if (docTime > 0) {
            long timeInSeconds = docTime;
            long timeInMs = timeInSeconds * 1000;
            RunCommandBuffer.insert(RunCommandBuffer.begin(), timeInMs);
            
            char tbs[60];
            sprintf(tbs, "* Added Run time: %lu ms", timeInMs);
            Serial.println(tbs);

            if (RunCommandBuffer.size() == 1) runCar(timeInMs);
        }
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