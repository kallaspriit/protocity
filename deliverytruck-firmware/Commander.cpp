#include "Commander.h"

Commander::Commander(Stream *serial, char startToken, char endToken, char parameterDelimiter, char escapeChar) :
  serial(serial),
  startToken(startToken),
  endToken(endToken),
  parameterDelimiter(parameterDelimiter),
  escapeChar(escapeChar),
  lastChar('0'),
  parameterCount(0),
  command("")
{
  
}

boolean Commander::gotCommand() {
  boolean gotCommand = false;
  
  while (serial->available() > 0) {
    char character = serial->read();
  
    if (character == startToken && lastChar != escapeChar) {
      buffer = "";
    } else if (character == endToken && lastChar != escapeChar) {
      parseCommand(buffer);
  
      buffer = "";
      gotCommand = true;
      
      lastChar = character;
      
      break;
    } else {
      if (character != escapeChar) {
        buffer += character;
      }
      
      lastChar = character;
    }
  }
  
  return gotCommand;
}

void Commander::parseCommand(String buffer) {
  parameterCount = 0;
  command = "";
  
  int lastDelimiterPos = 0;
  int delimitersFound = 0;
  
  while (true) {
    // make sure we don't exceed the parameter limit
    if (parameterCount >= COMMANDER_MAX_PARAMETER_COUNT - 1) {
      break;
    }
    
    int delimiterPos = buffer.indexOf(parameterDelimiter, lastDelimiterPos + 1);
    
    if (delimiterPos == -1) {
      // there are no parameters so the whole command is a name
      if (delimitersFound == 0) {
        command = buffer;
      }
      
      break;
    }
    
    delimitersFound++;
    
    // first one is the name and parameters follow
    if (delimitersFound == 1) {
      command = buffer.substring(0, delimiterPos);
    } else {
      parameters[parameterCount++] = buffer.substring(lastDelimiterPos + 1, delimiterPos);
    }
    
    lastDelimiterPos = delimiterPos;
  }
  
  // extract last parameter
  if (delimitersFound > 0) {
     parameters[parameterCount++] = buffer.substring(lastDelimiterPos + 1, buffer.length());
  }
}

