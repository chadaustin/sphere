#include <stdio.h>
#include "Log.hpp"


////////////////////////////////////////////////////////////////////////////////

CLog& operator<<(CLog& log, const char* string)
{
  log.WriteString(string);
  return log;
}

////////////////////////////////////////////////////////////////////////////////

CLog& operator<<(CLog& log, char character)
{
  char string[2] = { character, 0 };
  log.WriteString(string);
  return log;
}

////////////////////////////////////////////////////////////////////////////////

CLog& operator<<(CLog& log, int integer)
{
  char string[20];
  sprintf(string, "%d", integer);
  log.WriteString(string);
  return log;
}

////////////////////////////////////////////////////////////////////////////////
