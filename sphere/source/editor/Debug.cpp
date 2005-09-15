#include "Debug.hpp"

int LogBlock__::NumTabs;

CLogDispatcher DebugLog__;
extern CLogDispatcher DebugLog__;

// logs the entry and exit of a block of code, indenting any log strings in the body
#define LOG_BLOCK(name) LogBlock__ log__block__(name);

// logs one line
#define LOG(x) { for (int i = 0; i < LogBlock__::NumTabs; i++) DebugLog__ << "    "; DebugLog__ << x; }


  LogBlock__(const char* name)
  {
    m_Name = name;

    LOG('+' << m_Name.c_str() << '\n')
    NumTabs++;
  }

  ~LogBlock__()
  {
    NumTabs--;
    LOG('-' << m_Name.c_str() << '\n')
  }

////////////////////////////////////////////////////////////////////////////////

void InitializeLog()
{
  // DebugLog__.AddLog(new CLogFile("sde.log"));
}

////////////////////////////////////////////////////////////////////////////////
