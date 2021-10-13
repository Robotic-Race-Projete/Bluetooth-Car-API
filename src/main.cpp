#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <arduino-timer.h>
#include <vector>

// Macros

#define LEFT_WHEEL_PIN 7
#define RIGHT_WHEEL_PIN 6
#define LEFT_LDR_SENSOR_PIN A0
#define RIGHT_LDR_SENSOR_PIN A1
#define ARD_RX 2
#define ARD_TX 3

// Global

Timer<10> timer;
std::vector<long> RunCommandBuffer;

SoftwareSerial bluetooth(2, 3); // RX, TX

// Functions

bool shouldCarRun = false;

void activateMotor() { shouldCarRun = true; }
void deactivateMotor() { shouldCarRun = false; }

bool continueToRunIfMoreCommand(void *);
void runCar(long time)
{
    char tbs[60];
    sprintf(tbs, "> Starting Run for time: %lu ms", time);
    Serial.println(tbs);

    activateMotor();
    timer.in(time, continueToRunIfMoreCommand);
}

bool continueToRunIfMoreCommand(void *)
{
    Serial.println("> Finished a run");
    RunCommandBuffer.pop_back();
    if (RunCommandBuffer.size() > 0)
    {
        long time = RunCommandBuffer.at(RunCommandBuffer.size() - 1);
        runCar(time);
    }
    else
        deactivateMotor();

    return false;
}

// Command example:  CAR+RUN:15
void handleBluetoothCommands(String command)
{
    StaticJsonDocument<256> doc;
    deserializeJson(doc, command);
    const char *commandType = doc["command"];

    if (strcmp(commandType, "RUN") == 0)
    {
        long docTime = doc["time"];
        if (docTime >= 0)
        {
            long timeInSeconds = (docTime != 0) ? docTime : 10;
            long timeInMs = timeInSeconds * 1000;
            RunCommandBuffer.insert(RunCommandBuffer.begin(), timeInMs);

            char tbs[60];
            sprintf(tbs, "* Added Run time: %lu ms", timeInMs);
            Serial.println(tbs);

            if (RunCommandBuffer.size() == 1)
                runCar(timeInMs);
        }
    }
}

bool checkLdrForWheels(void *)
{
    int leftSensor = analogRead(LEFT_LDR_SENSOR_PIN);
    int rightSensor = analogRead(RIGHT_LDR_SENSOR_PIN);

    // Log
    char tbs[80];
    sprintf(tbs, "leftSensor: %d | rightSensor: %d;", leftSensor, rightSensor);
    Serial.println(tbs);

    int margin = 50;
    if (true)
    { // shouldCarRun) {
        bool bLeftWheel = false;
        bool bRightWheel = false;

        if (leftSensor - rightSensor > margin)
        { 
            bLeftWheel = true;
        }
        else if (rightSensor - leftSensor > margin)
        {
            bRightWheel = true;
        }
        else
        {
            bLeftWheel = true;
            bRightWheel = true;
        }

        digitalWrite(LEFT_WHEEL_PIN, bLeftWheel);
        digitalWrite(RIGHT_WHEEL_PIN, bRightWheel);
    }
    else
    {
        digitalWrite(LEFT_WHEEL_PIN, 0);
        digitalWrite(RIGHT_WHEEL_PIN, 0);
    }

    return true;
}

void setup()
{
    pinMode(7, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(9, OUTPUT); // key pin
    digitalWrite(9, HIGH);

    Serial.begin(9600);
    bluetooth.begin(9600);

    Serial.println("Starting serial comms..!");

    // Led
    pinMode(6, OUTPUT);
    timer.every(50, checkLdrForWheels);
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