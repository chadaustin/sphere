#include <stdio.h>
#include <jsapi.h>
#include "Scripting.hpp"


// reserved word list
static const char* Keywords[] = {

  // keywords
  "break", "else", "new", "var",
  "case", "finally", "return", "void",
  "catch", "for", "switch", "while",
  "continue", "function", "this", "with",
  "default", "if", "throw",
  "delete", "in", "try",
  "do", "instanceof", "typeof",

  // future reserved words
  "abstract", "enum", "int", "short",
  "boolean", "export", "interface", "static",
  "byte", "extends", "long", "super",
  "char", "final", "native", "synchronized",
  "class", "float", "package", "throws",
  "const", "goto", "private", "transient",
  "debugger", "implements", "protected", "volatile",
  "double", "import", "public",

  // null literal
  "null",

  // boolean literals
  "true", "false",
};


static void ErrorReporter(JSContext* cx, const char* message, JSErrorReport* report);


static char* s_Script;
static bool s_HasError = false;
static sCompileError s_Error;


////////////////////////////////////////////////////////////////////////////////

bool IsKeyword(const char* token)
{
  for (unsigned int i = 0; i < sizeof(Keywords) / sizeof(*Keywords); i++)
    if (strcmp(token, Keywords[i]) == 0)
      return true;
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool VerifyScript(const char* script, sCompileError& error)
{
  // compile the script (checks for syntax errors)

  int size = strlen(script);
  s_Script = new char[size + 1];
  strcpy(s_Script, script);
  
  JSRuntime* rt = JS_NewRuntime(4 * 1024 * 1024);
  if (rt == NULL) {
    delete[] s_Script;
    return false;
  }

  JSContext* cx = JS_NewContext(rt, 8 * 1024);
  if (cx == NULL) {
    JS_DestroyRuntime(rt);
    delete[] s_Script;
    return false;
  }

  JSClass global_class = {
    "global", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  };

  JSObject* global = JS_NewObject(cx, &global_class, NULL, NULL);
  if (global == NULL) {
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    delete[] s_Script;
    return false;
  }

  JS_InitStandardClasses(cx, global);
  JS_SetErrorReporter(cx, ErrorReporter);

  s_HasError = false;

  // time to actually compile
  JSScript* compiled_script = JS_CompileScript(cx, global, s_Script, size, "", 0);
  if (compiled_script) {
    JS_DestroyScript(cx, compiled_script);
  }

  JS_DestroyContext(cx);
  JS_DestroyRuntime(rt);

  if (s_HasError) {
    error = s_Error;
  }

  // we're done
  delete[] s_Script;
  s_Script = NULL;
  return !s_HasError;
}

////////////////////////////////////////////////////////////////////////////////

static std::string GetToken(const char* tokenptr)
{
  // hack
  return " ";
}

////////////////////////////////////////////////////////////////////////////////

static int GetTokenStart(int line, int offset)
{
  const char* p = s_Script;

  for (int i = 0; i < line; i++)
  {
    while (*p && *p != '\n')
      p++;
    p++;
  }

  return (p - s_Script) + offset;
}

////////////////////////////////////////////////////////////////////////////////

static void ErrorReporter(JSContext* cx, const char* message, JSErrorReport* report)
{
  s_HasError = true;

  // fill error information
  s_Error.m_Message = message;
  s_Error.m_Token = GetToken(report->tokenptr);
  s_Error.m_TokenLine = report->lineno;
  s_Error.m_TokenStart = GetTokenStart(report->lineno, report->tokenptr - report->linebuf);
}

////////////////////////////////////////////////////////////////////////////////
