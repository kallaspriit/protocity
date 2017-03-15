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
    virtual String getVersion() { return "2.26.0"; };

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
    void loopObstacleDetection(unsigned long deltaTime);
    void loopMotorController(unsigned long deltaTime);

    // handle events
    virtual void handleClientDisconnected();

    // handle commands
    virtual void handleCommand(int requestId, String command, String parameters[], int parameterCount);
    void handleSetSpeedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetSpeedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetObstacleDistanceCommand(int requestId, String parameters[], int parameterCount);

    // disable debug led functionality (pin used for charge detection)
    virtual void toggleDebugLed() {};
    virtual void setDebugLed(int state) {};

    // send state to client
    void sendMotorSpeed(int requestId);
    void sendObstacleDistance(int requestId);
    void sendObstacleDetectedEvent(float distance);
    void sendObstacleClearedEvent(int duration);

    // battery handling
    virtual float getBatteryVoltage();
    virtual BatteryChargeState getBatteryChargeState();

    // obstacle handling
    float getObstacleDistance();

    // motor handling
    void setMotorSpeed(int speed);
    void stopMotor();

    // pins config
    const int MOTOR_CONTROL_PIN_A = 0;
    const int ADC_SLAVE_SELECT_PIN = 2;
    const int MOTOR_CONTROL_PIN_B = 4;
    const int CHARGE_DETECTION_PIN = 5; // also used by builtin led, the led must be removed!

    // analog-to-digital converter config
    const float MAX_ADC_READ_VOLTAGE = 3.3f; // Vcc/Vref pin
    const int MAX_ADC_READ_VALUE = 4095;
    const int MAX_ANALOG_WRITE_VALUE = 1023;

    // obstacle detection config, apply some hysteresis
    const float OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM = 8.0f;
    const float OBSTACLE_CLEARED_DISTANCE_THRESHOLD_CM = OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM + 0.1f;
    const float OBSTACLE_DISTSNCE_CHANGED_THRESHOLD_CM = 0.5f;
    // const unsigned long OBSTACLE_DETECTED_THRESHOLD_DURATION = 50;
    const unsigned long OBSTACLE_DETECTED_THRESHOLD_DURATION = 0;

    // how often to run the main loop
    const unsigned long LOOP_INTERVAL = 10;

    // battery voltage detection config
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_1 = 8200.0f;       // between input and output
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_2 = 15000.0f;      // between input and ground
    const float BATTERY_VOLTAGE_CALIBRATION_MULTIPLIER = 0.99f;     // multimeter-measured voltage / reported voltage

    // battery voltage speed adjustment, power at full charge is 100%, power at 0% is the below %
    const int LOW_BATTERY_OUTPUT = 120;

    // dependencies
    MCP320X adc;

    // runtime info
    int motorSpeed = 0;
    int targetSpeed = 0;
    unsigned long lastLoopTime = 0;
    int obstacleDetectedDuration = 0;
    bool isObstacleDetected = false;
    float lastReportedObstacleDistance = -OBSTACLE_DISTSNCE_CHANGED_THRESHOLD_CM;
};

#endif
