#ifndef SCRIPTING_HPP
#define SCRIPTING_HPP

#pragma warning(disable: 4786)

#include <string>
#include <vector>


struct sCompileError
{
  sCompileError(
    const char* message = "",
    const char* token = "",
    int token_start = 0,
    int token_line = 0)
  : m_Message(message)
  , m_Token(token)
  , m_TokenStart(token_start)
  , m_TokenLine(token_line)
  {
  }

  std::string m_Message;
  std::string m_Token;
  int         m_TokenStart;
  int         m_TokenLine;
};


bool IsKeyword(const char* token);
bool VerifyScript(const char* script, sCompileError& error);

#endif
