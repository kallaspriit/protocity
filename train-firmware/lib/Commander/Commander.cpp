#include "Commander.hpp"

Commander::Commander() : isValid(false),
                         id(0),
                         parameterCount(0),
                         command("")
{
}

void Commander::parseCommand(String buffer)
{
  isValid = false;
  id = 0;
  command = "";
  parameterCount = 0;

  int lastDelimiterPos = -1;
  int tokenIndex = 0;

  while (true)
  {
    int delimiterPos = buffer.indexOf(':', lastDelimiterPos + 1);

    if (delimiterPos == -1)
    {
      break;
    }

    tokenIndex++;

    String token = buffer.substring(lastDelimiterPos + 1, delimiterPos);

    handleToken(tokenIndex, token);

    lastDelimiterPos = delimiterPos;
  }

  String token = buffer.substring(lastDelimiterPos + 1, buffer.length());
  tokenIndex++;

  handleToken(tokenIndex, token);
}

void Commander::handleToken(int tokenIndex, String token)
{
  if (tokenIndex == 1)
  {
    id = token.toInt();
  }
  else if (tokenIndex == 2)
  {
    command = token;

    isValid = true;
  }
  else
  {
    parameters[parameterCount++] = token;
  }
}
