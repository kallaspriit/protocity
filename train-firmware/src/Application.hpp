#ifndef APPLICATION_H
#define APPLICATION_H

#include <SocketApplication.hpp>
#include <MCP320X.hpp>
#include <SPI.h>

class Application : public SocketApplication {

public:
    Application(int port);

private:

    // provide version number
    virtual String getVersion() { return "2.13.0"; };

    // override main setup hooks
    virtual void setupBefore();
    virtual void setupGreeting();
    virtual void setupAfter();

    // setup resources
    void setupPinModes();
    void setupChargeDetection();
    void setupAdc();
    void setupMotorController();

    // provide custom loop functionality
    virtual void loopAfter();
    void loopMotorController();

    // handle events
    virtual void handleClientDisconnected();

    // handle commands
    virtual void handleCommand(int requestId, String command, String parameters[], int parameterCount);
    void handleSetSpeedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetSpeedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetObstacleDistanceCommand(int requestId, String parameters[], int parameterCount);

    // obstacle detection
    void handleObstacleDetected();
    void handleObstacleCleared();

    // disable debug led functionality (pin used for charge detection)
    virtual void toggleDebugLed() {};
    virtual void setDebugLed(int state) {};

    // send state to client
    void sendMotorSpeed(int requestId);
    void sendObstacleDistance(int requestId);
    void sendObstacleDetectedEvent(float distance);
    void sendObstacleClearedEvent();

    // battery handling
    virtual float getBatteryVoltage();
    virtual BatteryChargeState getBatteryChargeState();

    // obstacle handling
    float getObstacleDistance();
    bool isObstacleDetected();

    // motor handling
    void setMotorSpeed(int speed);
    void stopMotor();
    void brakeMotor();
    void applyMotorSpeed();

    // pins config
    const int MOTOR_CONTROL_PIN_A = 0;
    const int ADC_SLAVE_SELECT_PIN = 2;
    const int MOTOR_CONTROL_PIN_B = 4;
    const int CHARGE_DETECTION_PIN = 5; // also used by builtin led, the led must be removed!

    // analog-to-digital converter config
    const float MAX_ADC_READ_VOLTAGE = 3.3f; // Vcc/Vref pin
    const int MAX_ADC_READ_VALUE = 4095;
    const int MAX_ANALOG_WRITE_VALUE = 1023;

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
