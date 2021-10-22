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
int builtInLedVal = LOW;

SoftwareSerial bluetooth(2, 3); // RX, TX

bool shouldCarRun = false;
bool isTestEnabled = false;

// Functions

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
    else if (strcmp(commandType, "LED") == 0) {
        builtInLedVal = !builtInLedVal;
        digitalWrite(LED_BUILTIN, builtInLedVal);
    }
    else if (strcmp(commandType, "TEST") == 0) {
        bool value = doc["value"];
        isTestEnabled = value;
    }
    else if (strcmp(commandType, "WHEEL") == 0) {
        isTestEnabled = true;
        Serial.println("* Changing Motor");

        bool bLeftWheel = doc["left"];
        bool bRightWheel = doc["right"];

        char tbs[60];
        sprintf(tbs, "* Motor = left: %d, right: %d", bLeftWheel, bRightWheel);
        Serial.println(tbs);

        digitalWrite(LEFT_WHEEL_PIN, bLeftWheel);
        digitalWrite(RIGHT_WHEEL_PIN, bRightWheel);
    }
}

bool checkLdrForWheels(void *)
{
    if (isTestEnabled) return true; // validation

    int leftSensor = analogRead(LEFT_LDR_SENSOR_PIN);
    int rightSensor = analogRead(RIGHT_LDR_SENSOR_PIN);

    // Log
    char tbs[80];
    sprintf(tbs, "leftSensor: %d | rightSensor: %d;", leftSensor, rightSensor);
    Serial.println(tbs);

    int margin = 25;
    if (shouldCarRun)
    {
        bool bLeftWheel = false;
        bool bRightWheel = false;

        if (leftSensor - rightSensor > margin)
        { 
            bRightWheel = true;
        }
        else if (rightSensor - leftSensor > margin)
        {
            bLeftWheel = true;
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
    timer.every(20, checkLdrForWheels);
}

void loop()
{
    if (bluetooth.available())
    {
        String response = bluetooth.readString();
        Serial.println(response);
        handleBluetoothCommands(response);
    }
    if (Serial.available())
    {
        String response = Serial.readString();
        Serial.println(response);
        handleBluetoothCommands(response);
    }

    timer.tick();
}