#ifndef DEBUG_HPP
#define DEBUG_HPP


#include <string>
#include "../common/LogDispatcher.hpp"


extern CLogDispatcher DebugLog__;


// logs the entry and exit of a block of code, indenting any log strings in the body
#define LOG_BLOCK(name) LogBlock__ log__block__(name);

// logs one line
#define LOG(x) { for (int i = 0; i < LogBlock__::NumTabs; i++) DebugLog__ << "    "; DebugLog__ << x; }


class LogBlock__
{
public:
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

private:
  std::string m_Name;
  static int NumTabs;
};


extern void InitializeLog();


#endif
