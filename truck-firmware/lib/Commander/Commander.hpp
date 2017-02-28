#ifndef COMMANDER_H
#define COMMANDER_H

#include "Arduino.h"

#define COMMANDER_MAX_PARAMETER_COUNT 10

class Commander {
  
public:
  Commander();
  
  void parseCommand(String buffer);

  bool isValid;
  int id;
  String command;
  String parameters[COMMANDER_MAX_PARAMETER_COUNT];
  int parameterCount;

private:
  void handleToken(int tokenIndex, String token);
};

#endif
