#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <arduino-timer.h>
#include <vector>

Timer<10> timer;
std::vector<int> RunCommandBuffer;

void Run();

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

    RunCommandBuffer.pop_back();
    if (RunCommandBuffer.size() > 0) Run();

    return false;
}

void Run () {
    int time = RunCommandBuffer.at(RunCommandBuffer.size() - 1);

    Serial.print("Time: ");
    Serial.print(time);
    Serial.print("\n");

    activateMotor(NULL);
    timer.in(time, deactivateMotor);
}

// Command example:  CAR+RUN:15
void handleBluetoothCommands(String command)
{
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, command);
    const char* commandType = doc["command"];

    Serial.println(commandType);

    if (strcmp(commandType, "RUN") == 0) 
    {
        const int time = (doc["time"] ? doc["time"] : 10)*1000;
        RunCommandBuffer.insert(RunCommandBuffer.begin(), time);

        if (RunCommandBuffer.size() == 1) Run();
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