#ifndef COMMANDER_H
#define COMMANDER_H

#include "Arduino.h"

#define COMMANDER_MAX_PARAMETER_COUNT 10

class Commander {
  
public:
  Commander(Stream *serial, char startToken = '<', char endToken = '>', char parameterDelimiter = ':', char escapeChar = '\\');
  
  boolean gotCommand();
  void parseCommand(String buffer);
  
  String command;
  String parameters[COMMANDER_MAX_PARAMETER_COUNT];
  int parameterCount;

private:
  Stream *serial;
  char startToken;
  char endToken;
  char parameterDelimiter;
  char escapeChar;
  char lastChar;
  String buffer;
};

#endif
