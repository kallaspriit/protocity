#ifndef APPLICATION_H
#define APPLICATION_H

#include <SocketApplication.hpp>
#include <MCP320X.hpp>
#include <SPI.h>
#include <stdarg.h>

class Application : public SocketApplication {

public:
    Application(int port);

    virtual void setup();
    virtual void loop();

private:

    // provide version number
    virtual String getVersion() { return "2.3.0"; };

    // setup additional dependecies
    void setupPinModes();
    void setupAdc();
    void setupMotorController();

    // application-specific loop handlers
    void loopMotorController();

    // handle events
    virtual void handleClientDisconnected();

    // handle commands
    virtual bool handleCommand(int requestId, String command, String parameters[], int parameterCount);

    // handle application-specific commands
    void handleSetSpeedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetSpeedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetBatteryVoltageCommand(int requestId, String parameters[], int parameterCount);
    void handleGetObstacleDistanceCommand(int requestId, String parameters[], int parameterCount);

    // obstacle detection
    void handleObstacleDetected();
    void handleObstacleCleared();

    // send state to client
    void sendMotorSpeed(int requestId);
    void sendBatteryVoltage(int requestId);
    void sendObstacleDistance(int requestId);
    void sendObstacleDetectedEvent(float distance);
    void sendObstacleClearedEvent();

    // battery handling
    float getBatteryVoltage();
    float calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier);
    int getBatteryChargePercentage(float voltage);

    // obstacle handling
    float getObstacleDistance();
    bool isObstacleDetected();

    // motor handling
    void setMotorSpeed(int speed);
    void stopMotor();
    void brakeMotor();
    void applyMotorSpeed();

    // configure pins
    const int MOTOR_CONTROL_PIN_A    = 0;
    const int MOTOR_CONTROL_PIN_B    = 4;
    const int ADC_SLAVE_SELECT_PIN   = 2;

    // environment config
    const int ANALOG_OUT_RANGE = 1023;

    // analog-to-digital converter config
    const float MAX_ADC_READING_VOLTAGE = 3.3f; // Vcc/Vref pin
    const int MAX_ADC_READING_VALUE = 4095;

    // behaviour config
    const float OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM = 10.0f;
    const unsigned long SPEED_DECISION_INTERVAL = 10;
    const unsigned long BRAKE_DURATION = 250;

    // battery voltage detection config
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_1 = 8200.0f;       // between input and output
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_2 = 15000.0f;      // between input and ground
    const float BATTERY_VOLTAGE_CALIBRATION_MULTIPLIER = 0.99f;     // multimeter-measured voltage / reported voltage

    // dependencies
    MCP320X adc;

    // runtime info
    int motorSpeed = 0;
    int targetSpeed = 0;
    bool isBraking = false;
    bool stopAfterBrake = false;
    unsigned long brakeStartTime = 0;
    unsigned long lastSpeedDecisionTime = 0;
    float obstacleDistance = 0.0f;
    int obstacleDetectedFrames = 0;
    bool wasObstacleDetected = false;
};

#endif
