#include "Application.hpp"

Application::Application(int port) : SocketApplication(port),
                                     adc(ADC_SLAVE_SELECT_PIN)
{
}

void Application::setupBefore()
{
  setupPinModes();
}

void Application::setupGreeting()
{
  Serial.print(String("\n\n### Train v") + getVersion() + String(" ###\n"));
}

void Application::setupAfter()
{
  setupChargeDetection();
  setupAdc();
  setupMotorController();
}

void Application::setupPinModes()
{
  log("setting up pin-modes");

  pinMode(MOTOR_CONTROL_PIN_A, OUTPUT);
  pinMode(MOTOR_CONTROL_PIN_B, OUTPUT);
}

void Application::setupChargeDetection()
{
  log("setting up charge detection");

  pinMode(CHARGE_DETECTION_PIN, INPUT);
}

void Application::setupAdc()
{
  log("setting up analog to digital converter");

  SPI.begin();
  adc.begin();
}

void Application::setupMotorController()
{
  log("setting up motor controller");

  // make the motor stop by default
  stopMotor();
}

void Application::loopAfter()
{
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastLoopTime;

  if (deltaTime < LOOP_INTERVAL)
  {
    return;
  }

  lastLoopTime = currentTime;

  loopObstacleDetection(deltaTime);
  loopMotorController();
}

void Application::loopObstacleDetection(unsigned long deltaTime)
{
  float obstacleDistance = getObstacleDistance();
  float obstacleDistanceThreshold = isObstacleDetected ? obstacleClearedDistanceThreshold : obstacleDetectedDistanceThreshold;
  bool isObstacleNear = obstacleDistance < obstacleDistanceThreshold;

  // report new obstacle distance if it changes by considerable amount
  float reportedDistanceDiff = fabs(obstacleDistance - lastReportedObstacleDistance);

  if (isObstacleDetected && reportedDistanceDiff >= OBSTACLE_DISTANCE_CHANGED_THRESHOLD_CM)
  {
    sendEventMessage("obstacle-changed", String(obstacleDistance));

    lastReportedObstacleDistance = obstacleDistance;
  }

  if (isObstacleNear)
  {
    obstacleDetectedDuration += deltaTime;

    if (obstacleDetectedDuration >= OBSTACLE_DETECTED_THRESHOLD_DURATION && !isObstacleDetected)
    {
      isObstacleDetected = true;

      if (motorSpeed > 0)
      {
        log("obstacle detected at %s cm after %d ms, stopping the train from current speed of %d%%", String(obstacleDistance).c_str(), (int)obstacleDetectedDuration, motorSpeed);

        brake();
      }
      else
      {
        log("obstacle detected at %s cm after %d ms but the train is already stationary", String(obstacleDistance).c_str(), (int)obstacleDetectedDuration);
      }

      sendObstacleDetectedEvent(obstacleDistance);
    }
  }
  else if (isObstacleDetected)
  {
    if (motorSpeed != targetSpeed)
    {
      log("obstacle cleared at %s cm after %d ms, resuming target speed of %d%%", String(obstacleDistance).c_str(), (int)obstacleDetectedDuration, targetSpeed);

      setMotorSpeed(targetSpeed);
    }
    else
    {
      log("obstacle cleared at %s cm after %d ms", String(obstacleDistance).c_str(), (int)obstacleDetectedDuration);
    }

    sendObstacleClearedEvent((int)obstacleDetectedDuration);

    isObstacleDetected = false;
    obstacleDetectedDuration = 0;
  }
}

void Application::loopMotorController()
{
  if (isObstacleDetected)
  {
    return;
  }

  if (motorSpeed != targetSpeed)
  {
    log("applying target speed of %d%%", targetSpeed);

    setMotorSpeed(targetSpeed);
  }
}

void Application::handleClientDisconnected()
{
  SocketApplication::handleClientDisconnected();

  targetSpeed = 0;
}

void Application::handleCommand(int requestId, String command, String parameters[], int parameterCount)
{
  if (command == "set-speed")
  {
    handleSetSpeedCommand(requestId, parameters, parameterCount);
  }
  else if (command == "get-speed")
  {
    handleGetSpeedCommand(requestId, parameters, parameterCount);
  }
  else if (command == "get-obstacle-distance")
  {
    handleGetObstacleDistanceCommand(requestId, parameters, parameterCount);
  }
  else if (command == "set-obstacle-parameters")
  {
    handleSetObstacleParametersCommand(requestId, parameters, parameterCount);
  }
  else
  {
    handleUnsupportedCommand(requestId, command, parameters, parameterCount);
  }
}

void Application::handleSetSpeedCommand(int requestId, String parameters[], int parameterCount)
{
  if (parameterCount != 1)
  {
    return sendErrorMessage(requestId, "expected 1 parameter, for example '1:set-speed:50'");
  }

  targetSpeed = min(max(parameters[0].toInt(), -100), 100);

  sendMotorSpeed(requestId);
}

void Application::handleGetSpeedCommand(int requestId, String parameters[], int parameterCount)
{
  if (parameterCount != 0)
  {
    return sendErrorMessage(requestId, "expected no parameters, for example '1:get-speed'");
  }

  sendMotorSpeed(requestId);
}

void Application::handleGetObstacleDistanceCommand(int requestId, String parameters[], int parameterCount)
{
  if (parameterCount != 0)
  {
    return sendErrorMessage(requestId, "expected no parameters, for example '1:get-obstacle-distance'");
  }

  sendObstacleDistance(requestId);
}

void Application::handleSetObstacleParametersCommand(int requestId, String parameters[], int parameterCount)
{
  if (parameterCount != 3)
  {
    return sendErrorMessage(requestId, "expected 2 parameters, for example '1:set-obstacle-parameters:10.0:11.0:200'");
  }

  obstacleDetectedDistanceThreshold = parameters[0].toFloat();
  obstacleClearedDistanceThreshold = parameters[1].toFloat();
  brakeDuration = parameters[2].toInt();

  log("obstacle is now detected at %s cm and cleared at %s cm, train brakes for %dms", String(obstacleDetectedDistanceThreshold).c_str(), String(obstacleClearedDistanceThreshold).c_str(), brakeDuration);

  sendSuccessMessage(requestId);
}

void Application::sendMotorSpeed(int requestId)
{
  sendSuccessMessage(requestId, motorSpeed, targetSpeed);
}

void Application::sendObstacleDistance(int requestId)
{
  float distance = getObstacleDistance();

  sendSuccessMessage(requestId, String(distance));
}

void Application::sendObstacleDetectedEvent(float distance)
{
  sendEventMessage("obstacle-detected", String(distance));
}

void Application::sendObstacleClearedEvent(int duration)
{
  sendEventMessage("obstacle-cleared", String(duration));
}

float Application::getBatteryVoltage()
{
  int reading = adc.read12(adc.SINGLE_CH0);

  float actualVoltage = calculateAdcVoltage(
      reading,
      MAX_ADC_READ_VALUE,
      MAX_ADC_READ_VOLTAGE,
      BATTERY_VOLTAGE_DIVIDER_RESISTOR_1,
      BATTERY_VOLTAGE_DIVIDER_RESISTOR_2,
      BATTERY_VOLTAGE_CALIBRATION_MULTIPLIER);

  return actualVoltage;
}

Application::BatteryChargeState Application::getBatteryChargeState()
{
  int chargingState = digitalRead(CHARGE_DETECTION_PIN);

  if (chargingState == LOW)
  {
    return BatteryChargeState::CHARGE_STATE_NOT_CHARGING;
  }
  else
  {
    return BatteryChargeState::CHARGE_STATE_CHARGING;
  }
}

float Application::getObstacleDistance()
{
  int reading = adc.read12(adc.SINGLE_CH1);

  float voltage = calculateAdcVoltage(reading, MAX_ADC_READ_VALUE, MAX_ADC_READ_VOLTAGE, 0, 1, 1.0);
  float distance = max(min(13.0f * pow(voltage, -1), 30.0f), 4.0f);

  // log("obstacle distance: %s cm (reading: %d, voltage: %s)", String(distance).c_str(), reading, String(voltage).c_str());

  return distance;
}

void Application::setMotorSpeed(int speed)
{
  if (speed == motorSpeed)
  {
    return;
  }

  speed = min(max(speed, -100), 100);

  int analogOutValue = (int)(((float)abs(speed) / 100.0f) * (float)MAX_ANALOG_WRITE_VALUE);

  log("setting motor to %s at %d%% speed (%d)", speed > 0 ? "move forward" : speed < 0 ? "move in reverse" : "stop", speed, analogOutValue);

  motorSpeed = speed;

  // set outputs
  if (speed == 0)
  {
    analogWrite(MOTOR_CONTROL_PIN_A, MAX_ANALOG_WRITE_VALUE);
    analogWrite(MOTOR_CONTROL_PIN_B, MAX_ANALOG_WRITE_VALUE);
  }
  else if (speed > 0)
  {
    analogWrite(MOTOR_CONTROL_PIN_A, MAX_ANALOG_WRITE_VALUE);
    analogWrite(MOTOR_CONTROL_PIN_B, MAX_ANALOG_WRITE_VALUE - analogOutValue);
  }
  else
  {
    analogWrite(MOTOR_CONTROL_PIN_A, MAX_ANALOG_WRITE_VALUE - analogOutValue);
    analogWrite(MOTOR_CONTROL_PIN_B, MAX_ANALOG_WRITE_VALUE);
  }

  sendEventMessage("speed-changed", String(motorSpeed), String(targetSpeed));
}

void Application::stopMotor()
{
  setMotorSpeed(0);
}

void Application::brake()
{
  unsigned long currentTime = millis();
  unsigned long timeSinceLastBrake = currentTime - lastBrakeTime;

  if (timeSinceLastBrake < MIN_BRAKE_PAUSE)
  {
    // don't perform brake too often, just stop the motor
    log("brake requested too soon (%dms vs %dms threshold), ignoring it and just stopping the motor", timeSinceLastBrake, MIN_BRAKE_PAUSE);

    setMotorSpeed(0);
  }
  else
  {
    // apply full reverse for some time
    int direction = motorSpeed >= 0 ? -1 : 1;
    int brakeSpeed = direction * 100;

    log("braking at %d%% for %dms", brakeSpeed, brakeDuration);

    // brake for a duration, blocks the main thread but for a short time
    setMotorSpeed(brakeSpeed);
    delay(brakeDuration);
    setMotorSpeed(0);
  }

  lastBrakeTime = currentTime;
}

void Application::setupDebugLed()
{
  log("setting up debug led requested, ignoring it");
}

void Application::setDebugLed(int state)
{
  log("setting debug led to %s requested", state == HIGH ? "ON" : "OFF");
}
