#ifndef SCRIPTING_HPP
#define SCRIPTING_HPP

#pragma warning(disable: 4786)

#include <string>
#include <vector>

#include <jsapi.h>

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

extern bool s_ShouldExit;
extern bool s_IsRunning;

bool IsKeyword(const char* token);
bool VerifyScript(const char* script, sCompileError& error);

bool VerifyScript(const char* script, sCompileError& error, JSRuntime* rt, JSContext* cx, JSObject* global);
void ErrorReporter(JSContext* cx, const char* message, JSErrorReport* report);

JSBool BranchCallback(JSContext* cx, JSScript* script);

#endif
