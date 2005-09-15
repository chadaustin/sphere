#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <string>

#include "../common/LogDispatcher.hpp"

class LogBlock__
{
public:
  LogBlock__(const char* name);
  ~LogBlock__();

private:
  std::string m_Name;
  static int NumTabs;
};


extern void InitializeLog();

#endif
