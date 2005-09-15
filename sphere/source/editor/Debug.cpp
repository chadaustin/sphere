#include "Debug.hpp"

int LogBlock::s_NumTabs = 0;
CLogDispatcher __DebugLog__;

////////////////////////////////////////////////////////////////////////////////

LogBlock::LogBlock(const char* name)
{
  m_Name = name;

  LOG('+' << m_Name.c_str() << '\n')
  s_NumTabs++;
}

LogBlock::~LogBlock()
{
  s_NumTabs--;
  LOG('-' << m_Name.c_str() << '\n')
}

////////////////////////////////////////////////////////////////////////////////

void InitializeLog()
{
  // __DebugLog__.AddLog(new CLogFile("sde.log"));
}

////////////////////////////////////////////////////////////////////////////////
