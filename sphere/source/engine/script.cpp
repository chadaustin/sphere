#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <sstream>
#include <math.h>
#include "script.hpp"

#include "audio.hpp"
#include "inputx.hpp"
#include "log.hpp"
#include "map_engine.hpp"
#include "render.hpp"
#include "sfont.hpp"
#include "sspriteset.hpp"
#include "system.hpp"
#include "swindowstyle.hpp"
#include "time.hpp"

#include "../common/version.h"
#include "../common/configfile.hpp"
#include "../common/Entities.hpp"



const int MAX_RECURSE_COUNT = 256;
const int MAX_FRAME_SKIP    = 20;



const dword SS_SOCKET_MAGIC      = 0x70274be2;
const dword SS_LOG_MAGIC         = 0x435262c9;
const dword SS_COLOR_MAGIC       = 0x449b0beb;
const dword SS_SPRITESET_MAGIC   = 0x5f7ca182;
const dword SS_SOUND_MAGIC       = 0x7a5e371a;
const dword SS_FONT_MAGIC        = 0x7f7d79ef;
const dword SS_WINDOWSTYLE_MAGIC = 0x53f8d469;
const dword SS_IMAGE_MAGIC       = 0x168875d3;
const dword SS_SURFACE_MAGIC     = 0x09bbff98;
const dword SS_ANIMATION_MAGIC   = 0x4c4ba103;
const dword SS_FILE_MAGIC        = 0x672d369a;
const dword SS_RAWFILE_MAGIC     = 0x29bcd805;
const dword SS_BYTEARRAY_MAGIC   = 0x2295027f;


struct SS_OBJECT
{
  SS_OBJECT(dword m) : magic(m) { }
  dword magic;  // for object verification
};

#define BEGIN_SS_OBJECT(name)             \
  struct name : SS_OBJECT {               \
    name() : SS_OBJECT(name##_MAGIC) { }

#define END_SS_OBJECT() \
  };


BEGIN_SS_OBJECT(SS_SOCKET)
  NSOCKET socket;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_LOG)
  CLog* log;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_COLOR)
  RGBA color;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_SPRITESET)
  SSPRITESET* spriteset;
  JSObject* object;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_SOUND)
  ADR_STREAM sound;
  bool is_playing;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_FONT)
  bool destroy_me;
  SFONT* font;
  RGBA mask;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_WINDOWSTYLE)
  bool destroy_me;
  SWINDOWSTYLE* windowstyle;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_IMAGE)
  bool destroy_me;
  IMAGE image;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_SURFACE)
  CImage32* surface;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_ANIMATION)
  IAnimation* animation;
  RGBA*       frame;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_FILE)
  CConfigFile* file;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_RAWFILE)
  IFile* file;
END_SS_OBJECT()

BEGIN_SS_OBJECT(SS_BYTEARRAY)
  int   size;
  byte* array;
END_SS_OBJECT()


/////////////////
// CScriptCode //
/////////////////

////////////////////////////////////////////////////////////////////////////////

CScriptCode::CScriptCode(JSContext* context, JSObject* global, JSScript* script)
: m_Context(context)
, m_Global(global)
, m_Script(script)
, m_NumReferences(1)
{
  m_ScriptObject = JS_NewScriptObject(context, script);
  JS_AddRoot(context, &m_ScriptObject);
}

////////////////////////////////////////////////////////////////////////////////

CScriptCode::~CScriptCode()
{
  JS_RemoveRoot(m_Context, &m_ScriptObject);
  // script will be GC'd automatically
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptCode::AddRef()
{
  m_NumReferences++;
}

////////////////////////////////////////////////////////////////////////////////

void
CScriptCode::Release()
{
  if (--m_NumReferences == 0) {
    delete this;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CScriptCode::Execute(bool& should_exit)
{
  // get a handle to the parent CScript object
  CScript* script_object = (CScript*)JS_GetContextPrivate(m_Context);

  // update the system (handle quit messages if we must)
  UpdateSystem();

  // store old error message
//  std::string old_error = script_object->m_Error;

  // reset the error to nothing
//  script_object->m_Error = "";

  jsval rval;
  JSBool result = JS_ExecuteScript(m_Context, m_Global, m_Script, &rval);
  bool succeeded = (result == JS_TRUE);

  // if there is no error message, we succeeded
  if (script_object->m_Error == "") {
    succeeded = true;
  }

  // restore error message
//  script_object->m_Error = old_error;

  should_exit = script_object->m_ShouldExit;
  return succeeded;
}

////////////////////////////////////////////////////////////////////////////////



/////////////
// CScript //
/////////////

////////////////////////////////////////////////////////////////////////////////

CScript::CScript(IEngine* engine)
: m_Engine(engine)
, m_Runtime(NULL)
, m_Context(NULL)
, m_Global(NULL)
, m_GCCount(0)

, m_RecurseCount(0)
, m_ShouldRender(true)
, m_FrameRate(0)
, m_FramesSkipped(0)
, m_IdealTime(0)
{
  // create runtime
  m_Runtime = JS_NewRuntime(1024 * 1024);
  if (m_Runtime == NULL) {
    return;
  }

  // create context
  m_Context = JS_NewContext(m_Runtime, 16 * 1024);
  if (m_Context == NULL) {
    JS_DestroyRuntime(m_Runtime);
    m_Runtime = NULL;
  }

  // create global class
  static JSClass global_class = {
    "global", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  };
  
  // create global object
  m_Global = JS_NewObject(m_Context, &global_class, NULL, NULL);
  if (m_Global == NULL) {
    JS_DestroyContext(m_Context);
    m_Context = NULL;
    JS_DestroyRuntime(m_Runtime);
    m_Runtime = NULL;
    return;
  }

  // assign the script object to the context
  JS_SetContextPrivate(m_Context, this);

  JS_InitStandardClasses(m_Context, m_Global);
  JS_SetErrorReporter(m_Context, ErrorReporter);
  JS_SetBranchCallback(m_Context, BranchCallback);

  InitializeSphereFunctions();
  InitializeSphereConstants();
}

////////////////////////////////////////////////////////////////////////////////

CScript::~CScript()
{
  if (m_Runtime) {
    if (m_Context) {
      JS_DestroyContext(m_Context); 
      m_Context = NULL;
    }

    JS_DestroyRuntime(m_Runtime);
    m_Runtime = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CScript::EvaluateFile(const char* filename, IFileSystem& fs)
{
  // load the script
  IFile* file = fs.Open(filename, IFileSystem::read);
  if (file == NULL) {
    m_Error = "Could not load file '";
    m_Error += filename;
    m_Error += "'";
    return false;
  }

  // read file
  int size = file->Size();
  char* script = new char[size + 1];
  file->Read(script, size);
  script[size] = '\0';
  file->Close();

  // evaluate the read text
  bool result = Evaluate(script, filename);
  delete[] script;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool
CScript::Evaluate(const char* script, const char* filename)
{
  if (!m_Runtime) {
    m_Error = "JavaScript engine initialization failed";
    return false;
  }

  // update the system (handle quit messages if we must)
  UpdateSystem();

  m_Error = "";
  m_ShouldExit = false;

  // run the script
  jsval result;
  JSBool r = JS_EvaluateScript(
    m_Context,
    m_Global,
    script,
    strlen(script),
    (filename ? filename : NULL),
    0,
    &result);

  // if script execution failed but there is no error, Exit() was called
  if (r == JS_FALSE && m_Error == "") {
    return true;
  }

  return (r == JS_TRUE);
}

////////////////////////////////////////////////////////////////////////////////

CScriptCode*
CScript::Compile(const char* script)
{
  m_ShouldExit = false;

  JSScript* s = JS_CompileScript(
    m_Context,
    m_Global,
    script,
    strlen(script),
    NULL,
    0
  );

  if (!s) {
    return NULL;
  }

  return new CScriptCode(m_Context, m_Global, s);
}

////////////////////////////////////////////////////////////////////////////////

const char*
CScript::GetError()
{
  return m_Error.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void
CScript::InitializeSphereFunctions()
{
  static JSFunctionSpec functions[] = {
    #define SS_FUNCTION(name, numargs) { #name, ss##name, numargs, 0, 0 },
    #include "ss_functions.table"
    #undef SS_FUNCTION
    { 0, 0, 0, 0, 0 },
  };

  JS_DefineFunctions(m_Context, m_Global, functions);
}

////////////////////////////////////////////////////////////////////////////////

void
CScript::InitializeSphereConstants()
{
  static struct {
    const char* name;
    int         value;
  } constants[] = {
    
    // surface - setBlendMode constants

    { "REPLACE", CImage32::REPLACE },
    { "BLEND",   CImage32::BLEND   },

    // keyboard constants

#define KEY_CONSTANT(name) { #name, name },
    KEY_CONSTANT(KEY_ESCAPE)
    KEY_CONSTANT(KEY_F1)
    KEY_CONSTANT(KEY_F2)
    KEY_CONSTANT(KEY_F3)
    KEY_CONSTANT(KEY_F4)
    KEY_CONSTANT(KEY_F5)
    KEY_CONSTANT(KEY_F6)
    KEY_CONSTANT(KEY_F7)
    KEY_CONSTANT(KEY_F8)
    KEY_CONSTANT(KEY_F9)
    KEY_CONSTANT(KEY_F10)
    KEY_CONSTANT(KEY_F11)
    KEY_CONSTANT(KEY_F12)
    KEY_CONSTANT(KEY_TILDE)
    KEY_CONSTANT(KEY_0)
    KEY_CONSTANT(KEY_1)
    KEY_CONSTANT(KEY_2)
    KEY_CONSTANT(KEY_3)
    KEY_CONSTANT(KEY_4)
    KEY_CONSTANT(KEY_5)
    KEY_CONSTANT(KEY_6)
    KEY_CONSTANT(KEY_7)
    KEY_CONSTANT(KEY_8)
    KEY_CONSTANT(KEY_9)
    KEY_CONSTANT(KEY_MINUS)
    KEY_CONSTANT(KEY_EQUALS)
    KEY_CONSTANT(KEY_BACKSPACE)
    KEY_CONSTANT(KEY_TAB)
    KEY_CONSTANT(KEY_A)
    KEY_CONSTANT(KEY_B)
    KEY_CONSTANT(KEY_C)
    KEY_CONSTANT(KEY_D)
    KEY_CONSTANT(KEY_E)
    KEY_CONSTANT(KEY_F)
    KEY_CONSTANT(KEY_G)
    KEY_CONSTANT(KEY_H)
    KEY_CONSTANT(KEY_I)
    KEY_CONSTANT(KEY_J)
    KEY_CONSTANT(KEY_K)
    KEY_CONSTANT(KEY_L)
    KEY_CONSTANT(KEY_M)
    KEY_CONSTANT(KEY_N)
    KEY_CONSTANT(KEY_O)
    KEY_CONSTANT(KEY_P)
    KEY_CONSTANT(KEY_Q)
    KEY_CONSTANT(KEY_R)
    KEY_CONSTANT(KEY_S)
    KEY_CONSTANT(KEY_T)
    KEY_CONSTANT(KEY_U)
    KEY_CONSTANT(KEY_V)
    KEY_CONSTANT(KEY_W)
    KEY_CONSTANT(KEY_X)
    KEY_CONSTANT(KEY_Y)
    KEY_CONSTANT(KEY_Z)
    KEY_CONSTANT(KEY_SHIFT)
    KEY_CONSTANT(KEY_CTRL)
    KEY_CONSTANT(KEY_ALT)
    KEY_CONSTANT(KEY_SPACE)
    KEY_CONSTANT(KEY_OPENBRACE)
    KEY_CONSTANT(KEY_CLOSEBRACE)
    KEY_CONSTANT(KEY_SEMICOLON)
    KEY_CONSTANT(KEY_APOSTROPHE)
    KEY_CONSTANT(KEY_COMMA)
    KEY_CONSTANT(KEY_PERIOD)
    KEY_CONSTANT(KEY_SLASH)
    KEY_CONSTANT(KEY_BACKSLASH)
    KEY_CONSTANT(KEY_ENTER)
    KEY_CONSTANT(KEY_INSERT)
    KEY_CONSTANT(KEY_DELETE)
    KEY_CONSTANT(KEY_HOME)
    KEY_CONSTANT(KEY_END)
    KEY_CONSTANT(KEY_PAGEUP)
    KEY_CONSTANT(KEY_PAGEDOWN)
    KEY_CONSTANT(KEY_UP)
    KEY_CONSTANT(KEY_RIGHT)
    KEY_CONSTANT(KEY_DOWN)
    KEY_CONSTANT(KEY_LEFT)

    KEY_CONSTANT(MOUSE_LEFT)
    KEY_CONSTANT(MOUSE_MIDDLE)
    KEY_CONSTANT(MOUSE_RIGHT)
#undef KEY_CONSTANT

#define MAP_ENGINE_CONSTANT(c) { #c, CMapEngine::##c },

    MAP_ENGINE_CONSTANT(COMMAND_WAIT)
    MAP_ENGINE_CONSTANT(COMMAND_FACE_NORTH)
    MAP_ENGINE_CONSTANT(COMMAND_FACE_NORTHEAST)
    MAP_ENGINE_CONSTANT(COMMAND_FACE_EAST)
    MAP_ENGINE_CONSTANT(COMMAND_FACE_SOUTHEAST)
    MAP_ENGINE_CONSTANT(COMMAND_FACE_SOUTH)
    MAP_ENGINE_CONSTANT(COMMAND_FACE_SOUTHWEST)
    MAP_ENGINE_CONSTANT(COMMAND_FACE_WEST)
    MAP_ENGINE_CONSTANT(COMMAND_FACE_NORTHWEST)
    MAP_ENGINE_CONSTANT(COMMAND_MOVE_NORTH)
    MAP_ENGINE_CONSTANT(COMMAND_MOVE_EAST)
    MAP_ENGINE_CONSTANT(COMMAND_MOVE_SOUTH)
    MAP_ENGINE_CONSTANT(COMMAND_MOVE_WEST)

    MAP_ENGINE_CONSTANT(SCRIPT_ON_CREATE)
    MAP_ENGINE_CONSTANT(SCRIPT_ON_DESTROY)
    MAP_ENGINE_CONSTANT(SCRIPT_ON_ACTIVATE_TOUCH)
    MAP_ENGINE_CONSTANT(SCRIPT_ON_ACTIVATE_TALK)
    MAP_ENGINE_CONSTANT(SCRIPT_COMMAND_GENERATOR)

#undef MAP_ENGINE_CONSTANT
  };


  // define the constants
  for (int i = 0; i < sizeof(constants) / sizeof(*constants); i++) {
    JS_DefineProperty(
      m_Context,
      m_Global,
      constants[i].name,
      INT_TO_JSVAL(constants[i].value),
      JS_PropertyStub,
      JS_PropertyStub,
      JSPROP_READONLY | JSPROP_PERMANENT
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CScript::ReportMapEngineError(const char* header)
{
  std::string error = header;
  error += "\n";
  error += m_Engine->GetMapEngine()->GetErrorMessage();
  JS_ReportError(m_Context, "%s", error.c_str());
}

////////////////////////////////////////////////////////////////////////////////

void
CScript::ErrorReporter(JSContext* cx, const char* message, JSErrorReport* report)
{
  CScript* This = (CScript*)JS_GetContextPrivate(cx);

  if (report) {

    // build an error
    std::ostringstream os;
    os << "Script error in '" << (report->filename ? report->filename : "(no filename)") << "', line: " << report->lineno << std::endl;
    os << std::endl;
    os << message << std::endl;
    if (report->linebuf) {
      os << report->linebuf << std::endl;
    }
    This->m_Error = os.str();

  } else {
    
    This->m_Error = message;

  }

  This->m_ShouldExit = true;
}

////////////////////////////////////////////////////////////////////////////////

JSBool
CScript::BranchCallback(JSContext* cx, JSScript* script)
{
  CScript* This = (CScript*)JS_GetContextPrivate(cx);

  // handle garbage collection
  if (This->m_GCCount++ >= 1024) {
    // handle system events
    UpdateSystem();

    // garbage collect!
    JS_GC(cx);
    This->m_GCCount = 0;
  }

  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////


// system function definition macros and inline functions

#define begin_func(name, minargs)                                                                      \
  JSBool CScript::ss##name(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) {       \
    CScript* This = (CScript*)JS_GetContextPrivate(cx);                                                \
    if (argc < minargs) {                                                                              \
      JS_ReportError(cx, "%s called with less than %s parameters", #name, #minargs);                   \
      *rval = JSVAL_NULL;                                                                              \
      return JS_FALSE;                                                                                 \
    }                                                                                                  \
    int arg = 0;

#define end_func()  \
    return (This->m_ShouldExit ? JS_FALSE : JS_TRUE); \
  }


// parameter grabbing

inline int argInt(JSContext* cx, jsval arg)
{
  int32 i;
  if (JS_ValueToInt32(cx, arg, &i) == JS_FALSE) {
    return 0; // invalid integer
  }
  return i;
}

inline const char* argStr(JSContext* cx, jsval arg)
{
  JSString* str = JS_ValueToString(cx, arg);
  if (str) {
    const char* s = JS_GetStringBytes(str);
    return (s ? s : "");
  } else {
    return "";
  }
}

inline bool argBool(JSContext* cx, jsval arg)
{
  JSBool b;
  if (JS_ValueToBoolean(cx, arg, &b) == JS_FALSE) {
    return false;
  }
  return (b == JS_TRUE);
}

inline double argDouble(JSContext* cx, jsval arg)
{
  jsdouble d;
  JS_ValueToNumber(cx, arg, &d);
  return d;
}

inline RGBA argColor(JSContext* cx, jsval arg)
{
  if (!JSVAL_IS_OBJECT(arg)) {
    JS_ReportError(cx, "Invalid color object");
    return CreateRGBA(0, 0, 0, 0);
  }

  SS_COLOR* color = (SS_COLOR*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(arg));
  if (color == NULL) {
    JS_ReportError(cx, "Invalid color object");
    return CreateRGBA(0, 0, 0, 0);
  }

  if (color->magic != SS_COLOR_MAGIC) {
    JS_ReportError(cx, "Invalid color object");
    return CreateRGBA(0, 0, 0, 0);
  }

  return color->color;
}

inline CImage32* argSurface(JSContext* cx, jsval arg)
{
  if (!JSVAL_IS_OBJECT(arg)) {
    JS_ReportError(cx, "Invalid surface object");
    return NULL;
  }

  SS_SURFACE* surface = (SS_SURFACE*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(arg));
  if (surface == NULL) {
    JS_ReportError(cx, "Invalid surface object");
    return NULL;
  }

  if (surface->magic != SS_SURFACE_MAGIC) {
    JS_ReportError(cx, "Invalid surface object");
    return NULL;
  }

  return surface->surface;
}

inline SS_BYTEARRAY* argByteArray(JSContext* cx, jsval arg)
{
  if (!JSVAL_IS_OBJECT(arg)) {
    JS_ReportError(cx, "Invalid byte array object");
    return NULL;
  }

  SS_BYTEARRAY* array = (SS_BYTEARRAY*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(arg));
  if (array == NULL) {
    JS_ReportError(cx, "Invalid byte array object");
    return NULL;
  }

  if (array->magic != SS_BYTEARRAY_MAGIC) {
    JS_ReportError(cx, "Invalid byte array object");
    return NULL;
  }

  return array;
}


#define arg_int(name)        int name           = argInt(cx, argv[arg++]);                                          if (This->m_ShouldExit) return JS_FALSE
#define arg_str(name)        const char* name   = argStr(cx, argv[arg++]);                                          if (This->m_ShouldExit) return JS_FALSE
#define arg_bool(name)       bool name          = argBool(cx, argv[arg++]);                                         if (This->m_ShouldExit) return JS_FALSE
#define arg_double(name)     double name        = argDouble(cx, argv[arg++]);                                       if (This->m_ShouldExit) return JS_FALSE
#define arg_color(name)      RGBA name          = argColor(cx, argv[arg++]);                                        if (This->m_ShouldExit) return JS_FALSE
#define arg_surface(name)    CImage32* name     = argSurface(cx, argv[arg++]);   if (name == NULL) return JS_FALSE; if (This->m_ShouldExit) return JS_FALSE
#define arg_byte_array(name) SS_BYTEARRAY* name = argByteArray(cx, argv[arg++]); if (name == NULL) return JS_FALSE; if (This->m_ShouldExit) return JS_FALSE


// return values

#define return_int(expr)      *rval = INT_TO_JSVAL(expr)
#define return_bool(expr)     *rval = BOOLEAN_TO_JSVAL(expr)
#define return_object(expr)   *rval = OBJECT_TO_JSVAL(expr)
#define return_str(expr)      *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, expr))
#define return_str_n(expr, n) *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, expr, n))
#define return_double(expr)   *rval = DOUBLE_TO_JSVAL(JS_NewDouble(cx, expr))



// Sphere function implementations

////////////////////////////////////////////////////////////////////////////////

begin_func(EvaluateScript, 1)
  arg_str(name);

  // read script
  std::string text;
  if (!This->m_Engine->GetScriptText(name, text)) {
    JS_ReportError(cx, "EvaluateScript() failed: Could not load script '%s'", name);
    return JS_FALSE;
  }

  // increment the recursion count, checking for overflow
  This->m_RecurseCount++;
  if (This->m_RecurseCount > MAX_RECURSE_COUNT) {
    JS_ReportError(cx, "EvaluateScript() recursed too deeply");
    return JS_FALSE;
  }

  // execute!
  if (!JS_EvaluateScript(cx, This->m_Global, text.c_str(), text.length(), name, 0, rval)) {
    return JS_FALSE;
  }

  This->m_RecurseCount--;

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(EvaluateSystemScript, 1)
  arg_str(name);

  // read script
  std::string text;
  if (!This->m_Engine->GetSystemScript(name, text)) {
    JS_ReportError(cx, "EvaluateSystemScript() failed: Could not load script '%s'", name);
    return JS_FALSE;
  }

  // increment the recursion count, checking for overflow
  This->m_RecurseCount++;
  if (This->m_RecurseCount > MAX_RECURSE_COUNT) {
    JS_ReportError(cx, "EvaluateScript() recursed too deeply");
    return JS_FALSE;
  }

  // execute!
  if (!JS_EvaluateScript(cx, This->m_Global, text.c_str(), text.length(), name, 0, rval)) {
    return JS_FALSE;
  }

  This->m_RecurseCount--;

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GarbageCollect, 0)
  JS_GC(cx);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(CreateStringFromCode, 1)
  arg_int(code);
  jschar c[2] = { (jschar)code, 0 };
  *rval = STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, c));
  return JS_TRUE;
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetVersionString, 0)
  return_str(SPHERE_VERSION);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetGameList, 0)
  
  // get the list of games
  std::vector<Game> games;
  This->m_Engine->GetGameList(games);

  static JSClass clasp = {
    "object", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, 
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  };

  int array_size = games.size();
  jsval* array = new jsval[array_size];
  for (int i = 0; i < array_size; i++) {
    JSObject* element = JS_NewObject(cx, &clasp, NULL, NULL);
    array[i] = OBJECT_TO_JSVAL(element);
    
    jsval name_val = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, games[i].name.c_str()));
    jsval dir_val  = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, games[i].directory.c_str()));

    // define the 'name' property
    JS_DefineProperty(
      cx, element, "name", name_val,
      JS_PropertyStub, JS_PropertyStub,
      JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT
    );

    // define the 'directory' property
    JS_DefineProperty(
      cx, element, "directory", dir_val,
      JS_PropertyStub, JS_PropertyStub,
      JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT
    );
  }

  // create the array
  JSObject* object = JS_NewArrayObject(cx, array_size, array);
  delete[] array;
  return_object(object);

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(ExecuteGame, 1)
  arg_str(directory);

  // make sure resolution switches don't affect the parent game
  int old_width  = GetScreenWidth();
  int old_height = GetScreenHeight();

  This->m_Engine->ExecuteGame(directory);

  SwitchResolution(old_width, old_height);

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(Exit, 0)
  // don't report an error (there is none)
  This->m_ShouldExit = true;
  return JS_FALSE;
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(Abort, 1)
  arg_str(message);
  This->m_ShouldExit = true;
  JS_ReportError(cx, "%s", message);
  return JS_FALSE;
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(OpenLog, 1)
  arg_str(filename);
  CLog* log = This->m_Engine->OpenLog(filename);

  if (log == NULL) {
    JS_ReportError(cx, "OpenLog() failed: Could not open file '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateLogObject(cx, log));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(FlipScreen, 0)
  
  if (This->m_FrameRate == 0) {

    FlipScreen();
    ClearScreen();

  } else {

    // never skip more than MAX_FRAME_SKIP frames
    if (This->m_ShouldRender || This->m_FramesSkipped >= MAX_FRAME_SKIP) {
      FlipScreen();
      ClearScreen();

      This->m_FramesSkipped = 0;
    } else {
      This->m_FramesSkipped++;
    }

    if (GetTime() * This->m_FrameRate < (dword)This->m_IdealTime) {
      This->m_ShouldRender = true;
      
      // delay!
      while (GetTime() * This->m_FrameRate < (dword)This->m_IdealTime) {
      }

    } else {
      This->m_ShouldRender = false;
    }

    // update timing variables
    This->m_IdealTime += 1000;
    
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetClippingRectangle, 4)
  arg_int(x);
  arg_int(y);
  arg_int(w);
  arg_int(h);
  SetClippingRectangle(x, y, w, h);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(ApplyColorMask, 1)
  arg_color(c);
  if (This->m_ShouldRender) {
    ApplyColorMask(c);
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetFrameRate, 1)
  arg_int(fps);
  This->m_FrameRate = fps;
  This->m_IdealTime = GetTime() * fps + 1000;
  This->m_ShouldRender = true;
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetFrameRate, 0)
  return_int(This->m_FrameRate);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetScreenWidth, 0)
  return_int(GetScreenWidth());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetScreenHeight, 0)
  return_int(GetScreenHeight());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(Point, 3)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_color(c);

    DrawPoint(x, y, c);
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(Line, 5)
  if (This->m_ShouldRender) {
    arg_int(x1);
    arg_int(y1);
    arg_int(x2);
    arg_int(y2);
    arg_color(c);
  
    int x[2] = { x1, x2 };
    int y[2] = { y1, y2 };
    DrawLine(x, y, c);
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GradientLine, 6)
  if (This->m_ShouldRender) {
    arg_int(x1);
    arg_int(y1);
    arg_int(x2);
    arg_int(y2);
    arg_color(c1);
    arg_color(c2);
  
    int x[2] = { x1, x2 };
    int y[2] = { y1, y2 };
    RGBA c[2];
    c[0] = c1;
    c[1] = c2;
    DrawGradientLine(x, y, c);
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(Triangle, 7)
  if (This->m_ShouldRender) {
    arg_int(x1);
    arg_int(y1);
    arg_int(x2);
    arg_int(y2);
    arg_int(x3);
    arg_int(y3);
    arg_color(c);

    int x[3] = { x1, x2, x3 };
    int y[3] = { y1, y2, y3 };
    DrawTriangle(x, y, c);
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GradientTriangle, 7)
  if (This->m_ShouldRender) {
    arg_int(x1);
    arg_int(y1);
    arg_int(x2);
    arg_int(y2);
    arg_int(x3);
    arg_int(y3);
    arg_color(c1);
    arg_color(c2);
    arg_color(c3);

    int x[3] = { x1, x2, x3 };
    int y[3] = { y1, y2, y3 };
    RGBA c[3];
    c[0] = c1;
    c[1] = c2;
    c[2] = c3;
    DrawGradientTriangle(x, y, c);
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(Rectangle, 5)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_int(w);
    arg_int(h);
    arg_color(c);

    DrawRectangle(x, y, w, h, c);
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GradientRectangle, 5)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_int(w);
    arg_int(h);
    arg_color(c1);
    arg_color(c2);
    arg_color(c3);
    arg_color(c4);

    RGBA c[4];
    c[0] = c1;
    c[1] = c2;
    c[2] = c3;
    c[3] = c4;
    DrawGradientRectangle(x, y, w, h, c);
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(AreKeysLeft, 0)
  return_bool(AreKeysLeft());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetKey, 0)
  return_int(GetKey());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(IsKeyPressed, 1)
  arg_int(key);
  RefreshInput();
  return_bool(IsKeyPressed(key));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(IsAnyKeyPressed, 0)
  RefreshInput();
  return_bool(AnyKeyPressed());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetKeyString, 2)
  arg_int(key);
  arg_bool(shift);
  return_str(GetKeyString(key, shift));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetMousePosition, 2)
  arg_int(x);
  arg_int(y);
  SetMousePosition(x, y);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetMouseX, 0)
  return_int(GetMouseX());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetMouseY, 0)
  return_int(GetMouseY());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(IsMouseButtonPressed, 1)
  arg_int(button);
  return_int(IsMouseButtonPressed(button));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetTime, 0)
  return_int(GetTime());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetLocalName, 0)
  char name[256];
  GetLocalName(name, 256);
  return_str(name);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetLocalAddress, 0)
  char name[256];
  GetLocalAddress(name, 256);
  return_str(name);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(OpenAddress, 2)
  arg_str(name);
  arg_int(port);
  return_object(CreateSocketObject(cx, OpenAddress(name, port)));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(ListenOnPort, 1)
  arg_int(port);
  return_object(CreateSocketObject(cx, ListenOnPort(port)));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(CreateByteArray, 1)
  arg_int(size);
  return_object(CreateByteArrayObject(cx, size));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(CreateByteArrayFromString, 1)
  arg_str(str);
  return_object(CreateByteArrayObject(cx, strlen(str), str));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(CreateStringFromByteArray, 1)
  arg_byte_array(array);
  return_str_n((char*)array->array, array->size);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(CreateColor, 3)
  arg_int(r);
  arg_int(g);
  arg_int(b);

  // if alpha isn't specified, default to 255
  int a = 255; 
  if (argc == 4) {
    a = argInt(cx, argv[3]);
  }

  // range check
  if (r < 0) { r = 0; }  if (r > 255) { r = 255; }
  if (g < 0) { g = 0; }  if (g > 255) { g = 255; }
  if (b < 0) { b = 0; }  if (b > 255) { b = 255; }
  if (a < 0) { a = 0; }  if (a > 255) { a = 255; }

  return_object(CreateColorObject(cx, CreateRGBA(r, g, b, a)));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(BlendColors, 2)
  arg_color(a);
  arg_color(b);
  return_object(CreateColorObject(cx, CreateRGBA(
    (a.red   + b.red)   / 2,
    (a.green + b.green) / 2,
    (a.blue  + b.blue)  / 2,
    (a.alpha + b.alpha) / 2
  )));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(BlendColorsWeighted, 4)
  arg_color(a);
  arg_color(b);
  arg_double(w1);
  arg_double(w2);
  
  if (w1 + w2 == 0) {
    return_object(CreateColorObject(cx, CreateRGBA(0, 0, 0, 255)));
  } else {
    return_object(CreateColorObject(cx, CreateRGBA(
      (a.red   * w1 + b.red   * w2) / (w1 + w2),
      (a.green * w1 + b.green * w2) / (w1 + w2),
      (a.blue  * w1 + b.blue  * w2) / (w1 + w2),
      (a.alpha * w1 + b.alpha * w2) / (w1 + w2)
    )));
  }
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(MapEngine, 2)
  arg_str(map);
  arg_int(fps);

  if (!This->m_Engine->GetMapEngine()->Execute(map, fps)) {
    This->ReportMapEngineError("MapEngine() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(ChangeMap, 1)
  arg_str(map);

  if (!This->m_Engine->GetMapEngine()->ChangeMap(map)) {
    This->ReportMapEngineError("ChangeMap() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(ExitMapEngine, 0)
  
  if (!This->m_Engine->GetMapEngine()->Exit()) {
    This->ReportMapEngineError("ExitMapEngine() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(UpdateMapEngine, 0)

  if (!This->m_Engine->GetMapEngine()->Update()) {
    This->ReportMapEngineError("UpdateMapEngine() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetNumLayers, 0)

  int layers;
  if (!This->m_Engine->GetMapEngine()->GetNumLayers(layers)) {
    This->ReportMapEngineError("GetNumLayers() failed");
    return JS_FALSE;
  }

  return_int(layers);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetLayerWidth, 1)
  arg_int(layer);

  int width;
  if (!This->m_Engine->GetMapEngine()->GetLayerWidth(layer, width)) {
    This->ReportMapEngineError("GetLayerWidth() failed");
    return JS_FALSE;
  }

  return_int(width);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetLayerHeight, 1)
  arg_int(layer);

  int height;
  if (!This->m_Engine->GetMapEngine()->GetLayerHeight(layer, height)) {
    This->ReportMapEngineError("GetLayerHeight() failed");
    return JS_FALSE;
  }

  return_int(height);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(IsLayerVisible, 1)
  arg_int(layer);

  bool visible;
  if (!This->m_Engine->GetMapEngine()->IsLayerVisible(layer, visible)) {
    This->ReportMapEngineError("IsLayerVisible() failed");
    return JS_FALSE;
  }

  return_bool(visible);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetLayerVisible, 2)
  arg_int(layer);
  arg_bool(visible);

  if (!This->m_Engine->GetMapEngine()->SetLayerVisible(layer, visible)) {
    This->ReportMapEngineError("SetLayerVisible() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetNumTiles, 0)
  
  int tiles;
  if (!This->m_Engine->GetMapEngine()->GetNumTiles(tiles)) {
    This->ReportMapEngineError("GetNumTiles() failed");
    return JS_FALSE;
  }

  return_int(tiles);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetTile, 4)
  arg_int(x);
  arg_int(y);
  arg_int(layer);
  arg_int(tile);

  if (!This->m_Engine->GetMapEngine()->SetTile(x, y, layer, tile)) {
    This->ReportMapEngineError("SetTile() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetTile, 3)
  arg_int(x);
  arg_int(y);
  arg_int(layer);
  
  int tile;
  if (!This->m_Engine->GetMapEngine()->GetTile(x, y, layer, tile)) {
    This->ReportMapEngineError("GetTile() failed");
    return JS_FALSE;
  }

  return_int(tile);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetTileWidth, 0)

  int width;
  if (!This->m_Engine->GetMapEngine()->GetTileWidth(width)) {
    This->ReportMapEngineError("GetTileWidth() failed");
    return JS_FALSE;
  }

  return_int(width);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetTileHeight, 0)

  int height;
  if (!This->m_Engine->GetMapEngine()->GetTileHeight(height)) {
    This->ReportMapEngineError("GetTileHeight() failed");
    return JS_FALSE;
  }

  return_int(height);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(RenderMap, 0)

  if (!This->m_Engine->GetMapEngine()->RenderMap()) {
    This->ReportMapEngineError("RenderMap() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetColorMask, 2)
  arg_color(color);
  arg_int(num_frames);
  
  if (!This->m_Engine->GetMapEngine()->SetColorMask(color, num_frames)) {
    This->ReportMapEngineError("SetColorMask() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetDelayScript, 2)
  arg_int(num_frames);
  arg_str(script);

  if (!This->m_Engine->GetMapEngine()->SetDelayScript(num_frames, script)) {
    This->ReportMapEngineError("SetDelayScript() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(BindKey, 3)
  arg_int(key);
  arg_str(on_down);
  arg_str(on_up);

  if (!This->m_Engine->GetMapEngine()->BindKey(key, on_down, on_up)) {
    This->ReportMapEngineError("BindKey() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(UnbindKey, 1)
  arg_int(key);

  if (!This->m_Engine->GetMapEngine()->UnbindKey(key)) {
    This->ReportMapEngineError("UnbindKey() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(AttachInput, 1)
  arg_str(person);

  if (!This->m_Engine->GetMapEngine()->AttachInput(person)) {
    This->ReportMapEngineError("AttachInput() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(DetachInput, 0)

  if (!This->m_Engine->GetMapEngine()->DetachInput()) {
    This->ReportMapEngineError("DetachInput() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(IsInputAttached, 0)

  bool attached;
  if (!This->m_Engine->GetMapEngine()->IsInputAttached(attached)) {
    This->ReportMapEngineError("IsInputAttached() failed");
    return JS_FALSE;
  }

  return_bool(attached);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetInputPerson, 0)

  std::string person;
  if (!This->m_Engine->GetMapEngine()->GetInputPerson(person)) {
    This->ReportMapEngineError("GetInputPerson() failed");
    return JS_FALSE;
  }

  return_str(person.c_str());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetUpdateScript, 1)
  arg_str(script);

  if (!This->m_Engine->GetMapEngine()->SetUpdateScript(script)) {
    This->ReportMapEngineError("SetUpdateScript() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetRenderScript, 1)
  arg_str(script);

  if (!This->m_Engine->GetMapEngine()->SetRenderScript(script)) {
    This->ReportMapEngineError("SetRenderScript() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetLayerRenderer, 2)
  arg_int(layer);
  arg_str(script);

  if (!This->m_Engine->GetMapEngine()->SetLayerRenderer(layer, script)) {
    This->ReportMapEngineError("SetLayerRenderer() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetLayerAlpha, 2)
  arg_int(layer);
  arg_int(alpha);

  if (!This->m_Engine->GetMapEngine()->SetLayerAlpha(layer, alpha)) {
    This->ReportMapEngineError("SetLayerAlpha() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetLayerAlpha, 1)
  arg_int(layer);

  int alpha;
  if (!This->m_Engine->GetMapEngine()->GetLayerAlpha(layer, alpha)) {
    This->ReportMapEngineError("GetLayerAlpha() failed");
    return JS_FALSE;
  }

  return_int(alpha);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(AttachCamera, 1)
  arg_str(person);

  if (!This->m_Engine->GetMapEngine()->AttachCamera(person)) {
    This->ReportMapEngineError("AttachCamera() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(DetachCamera, 0)

  if (!This->m_Engine->GetMapEngine()->DetachCamera()) {
    This->ReportMapEngineError("DetachCamera() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(IsCameraAttached, 0)

  bool attached;
  if (!This->m_Engine->GetMapEngine()->IsCameraAttached(attached)) {
    This->ReportMapEngineError("IsCameraAttached() failed");
    return JS_FALSE;
  }

  return_bool(attached);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetCameraPerson, 0)
  
  std::string person;
  if (!This->m_Engine->GetMapEngine()->GetCameraPerson(person)) {
    This->ReportMapEngineError("GetCameraPerson() failed");
    return JS_FALSE;
  }

  return_str(person.c_str());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetCameraX, 1)
  arg_int(x);

  if (!This->m_Engine->GetMapEngine()->SetCameraX(x)) {
    This->ReportMapEngineError("SetCameraX() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetCameraY, 1)
  arg_int(y);

  if (!This->m_Engine->GetMapEngine()->SetCameraY(y)) {
    This->ReportMapEngineError("SetCameraY() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetCameraX, 0)

  int x;
  if (!This->m_Engine->GetMapEngine()->GetCameraX(x)) {
    This->ReportMapEngineError("GetCameraX() failed");
    return JS_FALSE;
  }

  return_int(x);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetCameraY, 0)

  int y;
  if (!This->m_Engine->GetMapEngine()->GetCameraY(y)) {
    This->ReportMapEngineError("GetCameraY() failed");
    return JS_FALSE;
  }

  return_int(y);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetPersonList, 0)

  // ask the map engine for the list of names
  std::vector<std::string> names;
  if (!This->m_Engine->GetMapEngine()->GetPersonList(names)) {
    This->ReportMapEngineError("GetNumPersons() failed");
    return JS_FALSE;
  }

  // create an array of JS strings with which to initialize the array
  jsval* valarray = new jsval[names.size()];
  for (int i = 0; i < names.size(); i++) {
    valarray[i] = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, names[i].c_str()));
  }

  // create the array object
  JSObject* array = JS_NewArrayObject(cx, names.size(), valarray);
  if (array == NULL) {
    JS_ReportError(cx, "Fatal Error!  JS_NewArrayObject() failed!");
    delete[] valarray;
    return JS_FALSE;
  }

  // delete our temporary jsval array and return the JS array
  delete[] valarray;
  return_object(array);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(CreatePerson, 3)
  arg_str(name);
  arg_str(spriteset);
  arg_bool(destroy_with_map);

  if (!This->m_Engine->GetMapEngine()->CreatePerson(name, spriteset, destroy_with_map)) {
    This->ReportMapEngineError("CreatePerson() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(DestroyPerson, 1)
  arg_str(name);

  if (!This->m_Engine->GetMapEngine()->DestroyPerson(name)) {
    This->ReportMapEngineError("DestroyPerson() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetPersonX, 2)
  arg_str(name);
  arg_int(x);

  if (!This->m_Engine->GetMapEngine()->SetPersonX(name, x)) {
    This->ReportMapEngineError("SetPersonX() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetPersonY, 2)
  arg_str(name);
  arg_int(y);

  if (!This->m_Engine->GetMapEngine()->SetPersonY(name, y)) {
    This->ReportMapEngineError("SetPersonY() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetPersonLayer, 2)
  arg_str(name);
  arg_int(layer);

  if (!This->m_Engine->GetMapEngine()->SetPersonLayer(name, layer)) {
    This->ReportMapEngineError("SetPersonLayer() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetPersonX, 1)
  arg_str(name);

  int x;
  if (!This->m_Engine->GetMapEngine()->GetPersonX(name, x)) {
    This->ReportMapEngineError("GetPersonX() failed");
    return JS_FALSE;
  }

  return_int(x);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetPersonY, 1)
  arg_str(name);

  int y;
  if (!This->m_Engine->GetMapEngine()->GetPersonY(name, y)) {
    This->ReportMapEngineError("GetPersonY() failed");
    return JS_FALSE;
  }

  return_int(y);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetPersonLayer, 1)
  arg_str(name);

  int layer;
  if (!This->m_Engine->GetMapEngine()->GetPersonLayer(name, layer)) {
    This->ReportMapEngineError("GetPersonLayer() failed");
    return JS_FALSE;
  }

  return_int(layer);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(FollowPerson, 3)
  arg_str(person);
  arg_str(leader);
  arg_int(pixels);

  if (!This->m_Engine->GetMapEngine()->FollowPerson(person, leader, pixels)) {
    This->ReportMapEngineError("FollowPerson() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetPersonDirection, 2)
  arg_str(name);
  arg_str(direction);

  if (!This->m_Engine->GetMapEngine()->SetPersonDirection(name, direction)) {
    This->ReportMapEngineError("SetPersonDirection() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetPersonDirection, 1)
  arg_str(name);

  std::string direction;
  if (!This->m_Engine->GetMapEngine()->GetPersonDirection(name, direction)) {
    This->ReportMapEngineError("GetPersonDirection() failed");
    return JS_FALSE;
  }

  return_str(direction.c_str());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetPersonFrame, 2)
  arg_str(name);
  arg_int(frame);

  if (!This->m_Engine->GetMapEngine()->SetPersonFrame(name, frame)) {
    This->ReportMapEngineError("SetPersonFrame() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetPersonFrame, 1)
  arg_str(name);

  int frame;
  if (!This->m_Engine->GetMapEngine()->GetPersonFrame(name, frame)) {
    This->ReportMapEngineError("GetPersonFrame() failed");
    return JS_FALSE;
  }

  return_int(frame);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetPersonScaleFactor, 3)
  arg_str(name);
  arg_double(scale_w);
  arg_double(scale_h);

  if (!This->m_Engine->GetMapEngine()->SetPersonScaleFactor(name, scale_w, scale_h)) {
    This->ReportMapEngineError("SetPersonScaleFactor() failed");
    return JS_FALSE;
  }

end_func()


////////////////////////////////////////////////////////////////////////////////

begin_func(SetPersonScaleAbsolute, 3)
  arg_str(name);
  arg_int(width);
  arg_int(height);

  if (!This->m_Engine->GetMapEngine()->SetPersonScaleAbsolute(name, width, height)) {
    This->ReportMapEngineError("SetPersonScaleAbsolute() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetPersonSpriteset, 1)
  arg_str(name);

  SSPRITESET* spriteset = This->m_Engine->GetMapEngine()->GetPersonSpriteset(name);
  if (spriteset == NULL) {
    JS_ReportError(cx, "Could not find person '%s'", name);
    return JS_FALSE;
  }

  spriteset->AddRef();

  return_object(CreateSpritesetObject(cx, spriteset));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetPersonScript, 3)
  arg_str(name);
  arg_int(which);
  arg_str(script);

  if (!This->m_Engine->GetMapEngine()->SetPersonScript(name, which, script)) {
    This->ReportMapEngineError("SetPersonScript() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(CallPersonScript, 2)
  arg_str(name);
  arg_int(which);

  if (!This->m_Engine->GetMapEngine()->CallPersonScript(name, which)) {
    This->ReportMapEngineError("CallPersonScript() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetCurrentPerson, 0)
  
  std::string person;
  if (!This->m_Engine->GetMapEngine()->GetCurrentPerson(person)) {
    This->ReportMapEngineError("GetCurrentPerson() failed");
    return JS_FALSE;
  }

  return_str(person.c_str());
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(QueuePersonCommand, 3)
  arg_str(name);
  arg_int(command);
  arg_bool(immediate);

  if (!This->m_Engine->GetMapEngine()->QueuePersonCommand(name, command, immediate)) {
    This->ReportMapEngineError("QueuePersonCommand() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(ClearPersonCommands, 1)
  arg_str(name);

  if (!This->m_Engine->GetMapEngine()->ClearPersonCommands(name)) {
    This->ReportMapEngineError("ClearPersonCommands() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(IsPersonObstructed, 3)
  arg_str(name);
  arg_int(x);
  arg_int(y);

  bool result;
  if (!This->m_Engine->GetMapEngine()->IsPersonObstructed(name, x, y, result)) {
    This->ReportMapEngineError("IsPersonObstructed() failed");
    return JS_FALSE;
  }

  return_bool(result);
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetTalkActivationKey, 1)
  arg_int(key);

  if (!This->m_Engine->GetMapEngine()->SetTalkActivationKey(key)) {
    This->ReportMapEngineError("SetTalkActivationKey() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(SetTalkDistance, 1)
  arg_int(pixels);

  if (!This->m_Engine->GetMapEngine()->SetTalkDistance(pixels)) {
    This->ReportMapEngineError("SetTalkDistance() failed");
    return JS_FALSE;
  }

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(LoadSpriteset, 1)
  arg_str(filename);

  // load spriteset
  SSPRITESET* spriteset = This->m_Engine->LoadSpriteset(filename);
  if (spriteset == NULL) {
    JS_ReportError(cx, "Could not load spriteset '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateSpritesetObject(cx, spriteset));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(LoadSound, 1)
  arg_str(filename);

  // load sound
  ADR_STREAM sound = This->m_Engine->LoadSound(filename);
  if (sound == NULL) {
    JS_ReportError(cx, "Could not load sound '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateSoundObject(cx, sound));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetSystemFont, 0)
  return_object(CreateFontObject(cx, This->m_Engine->GetSystemFont(), false));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(LoadFont, 1)
  arg_str(filename);

  // load font
  SFONT* font = This->m_Engine->LoadFont(filename);
  if (font == NULL) {
    JS_ReportError(cx, "Could not load font '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateFontObject(cx, font, true));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetSystemWindowStyle, 0)
  return_object(CreateWindowStyleObject(cx, This->m_Engine->GetSystemWindowStyle(), false));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(LoadWindowStyle, 1)
  arg_str(filename);

  // load window style
  SWINDOWSTYLE* windowstyle = This->m_Engine->LoadWindowStyle(filename);
  if (windowstyle == NULL) {
    JS_ReportError(cx, "Could not load windowstyle '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateWindowStyleObject(cx, windowstyle, true));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetSystemArrow, 0)
  return_object(CreateImageObject(cx, This->m_Engine->GetSystemArrow(), false));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetSystemUpArrow, 0)
  return_object(CreateImageObject(cx, This->m_Engine->GetSystemUpArrow(), false));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetSystemDownArrow, 0)
  return_object(CreateImageObject(cx, This->m_Engine->GetSystemDownArrow(), false));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(LoadImage, 1)
  arg_str(filename);

  // load image
  IMAGE image = This->m_Engine->LoadImage(filename);
  if (image == NULL) {
    JS_ReportError(cx, "Could not load image '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateImageObject(cx, image, true));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GrabImage, 4)
  arg_int(x);
  arg_int(y);
  arg_int(w);
  arg_int(h);

  // make sure we don't go off the screen

  if (x < 0) {
    w += x;
    x = 0;
  }

  if (y < 0) {
    h += y;
    y = 0;
  }

  if (x + w > GetScreenWidth()) {
    w = GetScreenWidth() - x;
  }

  if (y + h > GetScreenHeight()) {
    h = GetScreenHeight() - y;
  }

  return_object(CreateImageObject(cx, GrabImage(x, y, w, h), true));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(CreateSurface, 3)
  arg_int(w);
  arg_int(h);
  arg_color(c);

  CImage32* surface = new CImage32(w, h);
  surface->SetBlendMode(CImage32::REPLACE);
  surface->Rectangle(0, 0, w - 1, h - 1, c);
  surface->SetBlendMode(CImage32::BLEND);

  return_object(CreateSurfaceObject(cx, surface));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(LoadSurface, 1)
  arg_str(filename);

  // load surface
  CImage32* surface = This->m_Engine->LoadSurface(filename);
  if (surface == NULL) {
    JS_ReportError(cx, "Could not load surface '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateSurfaceObject(cx, surface));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GrabSurface, 4)
  arg_int(x);
  arg_int(y);
  arg_int(w);
  arg_int(h);

  // make sure we don't go off the screen

  if (x < 0) {
    w += x;
    x = 0;
  }

  if (y < 0) {
    h += y;
    y = 0;
  }

  if (x + w > GetScreenWidth()) {
    w = GetScreenWidth() - x;
  }

  if (y + h > GetScreenHeight()) {
    h = GetScreenHeight() - y;
  }

  // create surface and grab pixels from the backbuffer
  CImage32* surface = new CImage32(w, h);
  DirectGrab(x, y, w, h, surface->GetPixels());

  return_object(CreateSurfaceObject(cx, surface));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(LoadAnimation, 1)
  arg_str(filename);

  // load animation
  IAnimation* animation = This->m_Engine->LoadAnimation(filename);
  if (animation == NULL) {
    JS_ReportError(cx, "Could not load animation '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateAnimationObject(cx, animation));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(GetFileList, 0)

  const char* directory = "save";
  if (argc > 1) {
    directory = argStr(cx, argv[0]);
  }

  // enumerate the files and return an array of string objects

  // get list of files from engine
  std::vector<std::string> vs;
  This->m_Engine->GetFileList(directory, vs);

  // convert it to an array of jsvals
  jsval* js_vs = new jsval[vs.size()];
  for (int i = 0; i < vs.size(); i++) {
    js_vs[i] = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, vs[i].c_str()));
  }

  return_object(JS_NewArrayObject(cx, vs.size(), js_vs));

  delete[] js_vs;

end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(OpenFile, 1)
  arg_str(filename);

  // open file
  CConfigFile* file = This->m_Engine->OpenFile(filename);
  if (file == NULL) {
    JS_ReportError(cx, "Could not open file '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateFileObject(cx, file));
end_func()

////////////////////////////////////////////////////////////////////////////////

begin_func(OpenRawFile, 1)
  arg_str(filename);

  // open file
  IFile* file = This->m_Engine->OpenRawFile(filename);
  if (file == NULL) {
    JS_ReportError(cx, "Could not open raw file '%s'", filename);
    return JS_FALSE;
  }

  return_object(CreateRawFileObject(cx, file));
end_func()

////////////////////////////////////////////////////////////////////////////////



// OBJECTS

#define begin_finalizer(Object, name)                   \
  void CScript::name(JSContext* cx, JSObject* obj)      \
  {                                                     \
    CScript* This = (CScript*)JS_GetContextPrivate(cx); \
    Object* object = (Object*)JS_GetPrivate(cx, obj);
  
#define end_finalizer()           \
    delete object;                \
    JS_SetPrivate(cx, obj, NULL); \
  }

#define begin_property(Object, name)                                      \
  JSBool CScript::name(JSContext* cx, JSObject* obj, jsval id, jsval* vp) \
  {                                                                       \
    CScript* This = (CScript*)JS_GetContextPrivate(cx);                   \
    Object* object = (Object*)JS_GetPrivate(cx, obj);

#define end_property()                                \
    return (This->m_ShouldExit ? JS_FALSE : JS_TRUE); \
  }

#define begin_method(Object, name, minargs)                                                            \
  JSBool CScript::name(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) {           \
    CScript* This = (CScript*)JS_GetContextPrivate(cx);                                                \
    Object* object = (Object*)JS_GetPrivate(cx, obj);                                                  \
    if (object == NULL || object->magic != Object##_MAGIC) {                /* invalid object */       \
      JS_ReportError(cx, "%s called on invalid object", #name, #minargs);                              \
      return JS_FALSE;                                                                                 \
    }                                                                                                  \
    if (argc < minargs) {                                                                              \
      JS_ReportError(cx, "%s called with less than %s parameters", #name, #minargs);                   \
      return JS_FALSE;                                                                                 \
    }                                                                                                  \
    int arg = 0;

#define end_method()                                  \
    return (This->m_ShouldExit ? JS_FALSE : JS_TRUE); \
  }


////////////////////////////////////////
// SOCKET OBJECTS //////////////////////
////////////////////////////////////////

JSObject*
CScript::CreateSocketObject(JSContext* cx, NSOCKET socket)
{
  static JSClass clasp = {
    "log", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeSocket,
  };
  
  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign methods to the object
  static JSFunctionSpec fs[] = {
    { "isConnected",        ssSocketIsConnected,        0, 0, 0 },
    { "getPendingReadSize", ssSocketGetPendingReadSize, 0, 0, 0 },
    { "read",               ssSocketRead,               1, 0, 0 },
    { "write",              ssSocketWrite,              1, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // attach the log to this object
  SS_SOCKET* socket_object = new SS_SOCKET;
  socket_object->socket = socket;
  JS_SetPrivate(cx, object, socket_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_SOCKET, ssFinalizeSocket)
  if (object->socket) {
    CloseSocket(object->socket);
  }
end_finalizer()

////////////////////////////////////////

begin_method(SS_SOCKET, ssSocketIsConnected, 0)
  if (object->socket) {
    return_bool(IsConnected(object->socket));
  } else {
    return_bool(false);
  }
end_method()

////////////////////////////////////////

begin_method(SS_SOCKET, ssSocketGetPendingReadSize, 0)
  if (object->socket) {
    return_int(GetPendingReadSize(object->socket));
  } else {
    return_int(-1);
  }
end_method()

////////////////////////////////////////

begin_method(SS_SOCKET, ssSocketRead, 1)
  if (object->socket) {
    arg_int(size);

    if (size <= 0) {
      return_object(CreateByteArrayObject(cx, 0));
    } else {
      void* buffer = malloc(size);
      int read = SocketRead(object->socket, buffer, size);
      if (read < 0) { // error!
        JS_ReportError(cx, "socket.read() failed miserably!");
        free(buffer);
        return JS_FALSE;
      }

      JSObject* array_object = CreateByteArrayObject(cx, read, buffer);
      free(buffer);

      return_object(array_object);
    }
  }
end_method()

////////////////////////////////////////

begin_method(SS_SOCKET, ssSocketWrite, 1)
  if (object->socket) {
    arg_byte_array(array);
    SocketWrite(object->socket, array->array, array->size);
  }
end_method()

////////////////////////////////////////



////////////////////////////////////////
// LOG OBJECTS /////////////////////////
////////////////////////////////////////

JSObject*
CScript::CreateLogObject(JSContext* cx, CLog* log)
{
  // define image class
  static JSClass clasp = {
    "log", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeLog,
  };
  
  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign methods to the object
  static JSFunctionSpec fs[] = {
    { "write",      ssLogWrite,      2, 0, 0 },
    { "beginBlock", ssLogBeginBlock, 1, 0, 0 },
    { "endBlock",   ssLogEndBlock,   0, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // attach the log to this object
  SS_LOG* log_object = new SS_LOG;
  log_object->log = log;
  JS_SetPrivate(cx, object, log_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_LOG, ssFinalizeLog)
  This->m_Engine->CloseLog(object->log);
end_finalizer()

////////////////////////////////////////

begin_method(SS_LOG, ssLogWrite, 1)
  arg_str(text);
  object->log->Send(text);
end_method()

////////////////////////////////////////

begin_method(SS_LOG, ssLogBeginBlock, 1)
  arg_str(name);
  object->log->BeginBlock(name);
end_method();

////////////////////////////////////////

begin_method(SS_LOG, ssLogEndBlock, 0)
  object->log->EndBlock();
end_method()

////////////////////////////////////////



////////////////////////////////////////
// COLOR OBJECTS ///////////////////////
////////////////////////////////////////

JSObject*
CScript::CreateColorObject(JSContext* cx, RGBA color)
{
  // define color class
  static JSClass clasp = {
    "color", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeColor,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  };
  
  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign properties
  static JSPropertySpec ps[] = {
    { "red",   0, JSPROP_PERMANENT, ssColorGetProperty, ssColorSetProperty },
    { "green", 1, JSPROP_PERMANENT, ssColorGetProperty, ssColorSetProperty },
    { "blue",  2, JSPROP_PERMANENT, ssColorGetProperty, ssColorSetProperty },
    { "alpha", 3, JSPROP_PERMANENT, ssColorGetProperty, ssColorSetProperty },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineProperties(cx, object, ps);

  // attach the color to this object
  SS_COLOR* color_object = new SS_COLOR;
  color_object->color = color;
  JS_SetPrivate(cx, object, color_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_COLOR, ssFinalizeColor)
end_finalizer()

////////////////////////////////////////

begin_property(SS_COLOR, ssColorGetProperty)
  int prop_id = argInt(cx, id);
  switch (prop_id) {
    case 0:  *vp = INT_TO_JSVAL(object->color.red);   break;
    case 1:  *vp = INT_TO_JSVAL(object->color.green); break;
    case 2:  *vp = INT_TO_JSVAL(object->color.blue);  break;
    case 3:  *vp = INT_TO_JSVAL(object->color.alpha); break;
    default: *vp = JSVAL_NULL;                        break;
  }
end_property()

////////////////////////////////////////

begin_property(SS_COLOR, ssColorSetProperty)
  int prop_id = argInt(cx, id);
  const char* name = "";
  switch (prop_id) {
    case 0: object->color.red   = argInt(cx, *vp); break;
    case 1: object->color.green = argInt(cx, *vp); break;
    case 2: object->color.blue  = argInt(cx, *vp); break;
    case 3: object->color.alpha = argInt(cx, *vp); break;
  }
end_property()

////////////////////////////////////////



////////////////////////////////////////
// SPRITESET OBJECTS ///////////////////
////////////////////////////////////////

JSObject*
CScript::CreateSpritesetObject(JSContext* cx, SSPRITESET* spriteset)
{
  // define class
  static JSClass clasp = {
    "spriteset", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeSpriteset,
  };

  static JSClass direction_clasp = {
    "direction", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  };

  static JSClass frame_clasp = {
    "frame", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  };

  
  // create object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  
  // define the member images array
  int num_images = spriteset->GetSpriteset().GetNumImages();
  jsval* image_values = new jsval[num_images];

  for (int i = 0; i < num_images; i++) {
    JSObject* image = CreateImageObject(cx, spriteset->GetImage(i), false);
    
    // define a reference back to this spriteset
    // we need this, because the spriteset owns the images, and we don't want them
    // to be GC'd while there is an active reference to this image
    JS_DefineProperty(
      cx,
      image,
      "spriteset",
      OBJECT_TO_JSVAL(object),
      JS_PropertyStub,
      JS_PropertyStub,
      JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT
    );

    image_values[i] = OBJECT_TO_JSVAL(image);
  }
  
  JSObject* image_array = JS_NewArrayObject(cx, num_images, image_values);
  delete[] image_values;


  // define the member directions array
  int num_directions = spriteset->GetSpriteset().GetNumDirections();
  jsval* direction_values = new jsval[num_directions];

  for (int i = 0; i < num_directions; i++) {
    JSObject* direction = JS_NewObject(cx, &direction_clasp, NULL, NULL);

    // set name property
    JS_DefineProperty(
      cx,
      direction,
      "name",
      STRING_TO_JSVAL(JS_NewStringCopyZ(cx, spriteset->GetSpriteset().GetDirectionName(i))),
      JS_PropertyStub,
      JS_PropertyStub,
      JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT
    );

    // create the frames array
    int num_frames = spriteset->GetSpriteset().GetNumFrames(i);
    jsval* frame_values = new jsval[num_frames];

    for (int j = 0; j < num_frames; j++) {
      JSObject* frame_object = JS_NewObject(cx, &frame_clasp, NULL, NULL);
      JS_DefineProperty(cx, frame_object, "index", INT_TO_JSVAL(spriteset->GetSpriteset().GetFrameIndex(i, j)), JS_PropertyStub, JS_PropertyStub, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
      JS_DefineProperty(cx, frame_object, "delay", INT_TO_JSVAL(spriteset->GetSpriteset().GetFrameDelay(i, j)), JS_PropertyStub, JS_PropertyStub, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
      frame_values[j] = OBJECT_TO_JSVAL(frame_object);
    }

    JS_DefineProperty(
      cx,
      direction,
      "frames",
      OBJECT_TO_JSVAL(JS_NewArrayObject(cx, num_frames, frame_values)),
      JS_PropertyStub,
      JS_PropertyStub,
      JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT
    );

    delete[] frame_values;

    direction_values[i] = OBJECT_TO_JSVAL(direction);
  }

  JSObject* direction_array = JS_NewArrayObject(cx, num_directions, direction_values);
  delete[] direction_values;

  
  // define the properties for this object
  JS_DefineProperty(
    cx,
    object,
    "images",
    OBJECT_TO_JSVAL(image_array),
    JS_PropertyStub,
    JS_PropertyStub,
    JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT
  );

  JS_DefineProperty(
    cx,
    object,
    "directions",
    OBJECT_TO_JSVAL(direction_array),
    JS_PropertyStub,
    JS_PropertyStub,
    JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT
  );

  // attach the spriteset to this object
  spriteset->AddRef();
  SS_SPRITESET* spriteset_object = new SS_SPRITESET;
  spriteset_object->spriteset = spriteset;
  spriteset_object->object    = object;
  JS_SetPrivate(cx, object, spriteset_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_SPRITESET, ssFinalizeSpriteset)
  // destroy the spriteset
  //This->m_Engine->DestroySpriteset(object->spriteset);
  object->spriteset->Release();
end_finalizer()

////////////////////////////////////////



////////////////////////////////////////
// SOUND OBJECTS ///////////////////////
////////////////////////////////////////

JSObject*
CScript::CreateSoundObject(JSContext* cx, ADR_STREAM sound)
{
  // define class
  static JSClass clasp = {
    "sound", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeSound,
  };
  
  // create object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign methods to the object
  static JSFunctionSpec fs[] = {
    { "play",        ssSoundPlay,        1, 0, 0 },
    { "stop",        ssSoundStop,        0, 0, 0 },
    { "setVolume",   ssSoundSetVolume,   1, 0, 0 },
    { "getVolume",   ssSoundGetVolume,   0, 0, 0 },
    { "setPan",      ssSoundSetPan,      1, 0, 0 },
    { "setVolume",   ssSoundGetPan,      0, 0, 0 },
    { "isPlaying",   ssSoundIsPlaying,   0, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // attach the sound to this object
  SS_SOUND* sound_object = new SS_SOUND;
  sound_object->sound = sound;
  JS_SetPrivate(cx, object, sound_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_SOUND, ssFinalizeSound)
  AdrCloseStream(object->sound);
end_finalizer()

////////////////////////////////////////

begin_method(SS_SOUND, ssSoundPlay, 1)
  arg_bool(repeat);
  AdrSetStreamRepeat(object->sound, (repeat ? ADR_TRUE : ADR_FALSE));
  AdrPlayStream(object->sound);
  object->is_playing = true;
end_method()

////////////////////////////////////////

begin_method(SS_SOUND, ssSoundStop, 0)
  AdrPauseStream(object->sound);
  AdrResetStream(object->sound);
  object->is_playing = false;
end_method()

////////////////////////////////////////

begin_method(SS_SOUND, ssSoundSetVolume, 1)
  arg_int(volume);
  AdrSetStreamVolume(object->sound, volume);
end_method()

////////////////////////////////////////

begin_method(SS_SOUND, ssSoundGetVolume, 0)
  return_int(AdrGetStreamVolume(object->sound));
end_method()

////////////////////////////////////////

begin_method(SS_SOUND, ssSoundSetPan, 1)
  arg_int(pan);
  AdrSetStreamPan(object->sound, pan);
end_method()

////////////////////////////////////////

begin_method(SS_SOUND, ssSoundGetPan, 0)
  return_int(AdrGetStreamPan(object->sound));
end_method()

////////////////////////////////////////

begin_method(SS_SOUND, ssSoundIsPlaying, 0)
  return_bool(object->is_playing);
end_method()

////////////////////////////////////////



////////////////////////////////////////
// FONT OBJECTS ////////////////////////
////////////////////////////////////////

JSObject*
CScript::CreateFontObject(JSContext* cx, SFONT* font, bool destroy)
{
  // define class
  static JSClass clasp = {
    "font", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeFont,
  };
  
  // create object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign methods to the object
  static JSFunctionSpec fs[] = {
    { "setColorMask",   ssFontSetColorMask,   1, 0, 0 },
    { "drawText",       ssFontDrawText,       3, 0, 0 },
    { "drawZoomedText", ssFontDrawZoomedText, 4, 0, 0 },
    { "drawTextBox",    ssFontDrawTextBox,    6, 0, 0 },
    { "getHeight",      ssFontGetHeight,      0, 0, 0 },
    { "getStringWidth", ssFontGetStringWidth, 1, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // attach the sound to this object
  SS_FONT* font_object = new SS_FONT;
  font_object->font       = font;
  font_object->destroy_me = destroy;
  font_object->mask       = CreateRGBA(255, 255, 255, 255);
  JS_SetPrivate(cx, object, font_object);

  return object;
}

///////////////////////////////////////

begin_finalizer(SS_FONT, ssFinalizeFont)
  if (object->destroy_me) {
    delete object->font;
  }
end_finalizer()

///////////////////////////////////////

begin_method(SS_FONT, ssFontSetColorMask, 1)
  arg_color(mask);
  object->mask = mask;
end_method()

///////////////////////////////////////

begin_method(SS_FONT, ssFontDrawText, 3)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_str(text);
    object->font->DrawString(x, y, text, object->mask);
  }
end_method()

///////////////////////////////////////

begin_method(SS_FONT, ssFontDrawZoomedText, 4)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_double(scale);
    arg_str(text);

    object->font->DrawZoomedString(x, y, scale, text, object->mask);
  }
end_method()

///////////////////////////////////////

begin_method(SS_FONT, ssFontDrawTextBox, 6)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_int(w);
    arg_int(h);
    arg_int(offset);
    arg_str(text);
    object->font->DrawTextBox(x, y, w, h, offset, text, object->mask);
  }
end_method()

///////////////////////////////////////

begin_method(SS_FONT, ssFontGetHeight, 0)
  return_int(object->font->GetMaxHeight());
end_method()

///////////////////////////////////////

begin_method(SS_FONT, ssFontGetStringWidth, 1)
  arg_str(text);
  return_int(object->font->GetStringWidth(text));
end_method()

///////////////////////////////////////



///////////////////////////////////////
// WINDOW STYLE OBJECTS ///////////////
///////////////////////////////////////

JSObject*
CScript::CreateWindowStyleObject(JSContext* cx, SWINDOWSTYLE* ws, bool destroy)
{
  // define class
  static JSClass clasp = {
    "windowstyle", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeWindowStyle,
  };
  
  // create object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign methods to the object
  static JSFunctionSpec fs[] = {
    { "drawWindow", ssWindowStyleDrawWindow, 4, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // attach the window style to this object
  SS_WINDOWSTYLE* ws_object = new SS_WINDOWSTYLE;
  ws_object->windowstyle = ws;
  ws_object->destroy_me  = destroy;
  JS_SetPrivate(cx, object, ws_object);

  return object;
}

///////////////////////////////////////

begin_finalizer(SS_WINDOWSTYLE, ssFinalizeWindowStyle)
  if (object->destroy_me) {
    delete object->windowstyle;
  }
end_finalizer()

///////////////////////////////////////

begin_method(SS_WINDOWSTYLE, ssWindowStyleDrawWindow, 4)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_int(w);
    arg_int(h);

    object->windowstyle->DrawWindow(x, y, w, h);
  }
end_method()

///////////////////////////////////////



///////////////////////////////////////
// IMAGE OBJECTS //////////////////////
///////////////////////////////////////

JSObject*
CScript::CreateImageObject(JSContext* cx, IMAGE image, bool destroy)
{
  // define image class
  static JSClass clasp = {
    "image", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeImage,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  };
  
  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign methods to the object
  static JSFunctionSpec fs[] = {
    { "blit",              ssImageBlit,              2, 0, 0 },
    { "blitMask",          ssImageBlitMask,          3, 0, 0 },
    { "rotateBlit",        ssImageRotateBlit,        3, 0, 0 },
    { "zoomBlit",          ssImageZoomBlit,          3, 0, 0 },
    { "transformBlit",     ssImageTransformBlit,     8, 0, 0 },
    { "transformBlitMask", ssImageTransformBlitMask, 9, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // define width and height properties
  JS_DefineProperty(cx, object, "width",  INT_TO_JSVAL(GetImageWidth(image)),  JS_PropertyStub, JS_PropertyStub, JSPROP_READONLY | JSPROP_PERMANENT);
  JS_DefineProperty(cx, object, "height", INT_TO_JSVAL(GetImageHeight(image)), JS_PropertyStub, JS_PropertyStub, JSPROP_READONLY | JSPROP_PERMANENT);

  // attach the image to this object
  SS_IMAGE* image_object = new SS_IMAGE;
  image_object->image       = image;
  image_object->destroy_me  = destroy;
  JS_SetPrivate(cx, object, image_object);

  return object;
}

////////////////////////////////////////////////////////////////////////////////

begin_finalizer(SS_IMAGE, ssFinalizeImage)
  if (object->destroy_me) {
    DestroyImage(object->image);
  }
end_finalizer()

///////////////////////////////////////

begin_method(SS_IMAGE, ssImageBlit, 2)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    BlitImage(object->image, x, y);
  }
end_method()

///////////////////////////////////////

begin_method(SS_IMAGE, ssImageBlitMask, 3)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_color(clr);
    BlitImageMask(object->image, x, y, clr);
  }
end_method()

///////////////////////////////////////

begin_method(SS_IMAGE, ssImageRotateBlit, 3)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_double(radians);

    int w = GetImageWidth(object->image);
    int h = GetImageHeight(object->image);
    double r = sqrt(w * w + h * h) / 2;

    // various useful angles
    const double PI = 3.1415927;
    const double angle = atan((double)w / h);  // h shouldn't be zero...
    double upper_left_angle  = -angle;
    double upper_right_angle = angle;
    double lower_right_angle = PI - angle;
    double lower_left_angle  = PI + angle;

    // center of the image
    int cx = x + w / 2;
    int cy = y + h / 2;

    int tx[4] = {
      cx + r * sin(upper_left_angle  + radians),
      cx + r * sin(upper_right_angle + radians),
      cx + r * sin(lower_right_angle + radians),
      cx + r * sin(lower_left_angle  + radians),
    };

    // I'm not sure why we're doing subtraction here...
    // one of those "just smile and nod" things
    int ty[4] = {
      cy - r * cos(upper_left_angle  + radians),
      cy - r * cos(upper_right_angle + radians),
      cy - r * cos(lower_right_angle + radians),
      cy - r * cos(lower_left_angle  + radians),
    };

    TransformBlitImage(object->image, tx, ty);
  }

end_method()

///////////////////////////////////////

begin_method(SS_IMAGE, ssImageZoomBlit, 3)
  if (This->m_ShouldRender) {
    arg_int(x);
    arg_int(y);
    arg_double(factor);

    int w = GetImageWidth(object->image);
    int h = GetImageHeight(object->image);

    int tx[4] = { x, x + (int)(w * factor), x + (int)(w * factor), x };
    int ty[4] = { y, y, y + (int)(h * factor), y + (int)(h * factor) };

    TransformBlitImage(object->image, tx, ty);
  }
end_method()

///////////////////////////////////////

begin_method(SS_IMAGE, ssImageTransformBlit, 8)
  if (This->m_ShouldRender) {
    arg_int(x1);
    arg_int(y1);
    arg_int(x2);
    arg_int(y2);
    arg_int(x3);
    arg_int(y3);
    arg_int(x4);
    arg_int(y4);

    int x[4] = { x1, x2, x3, x4 };
    int y[4] = { y1, y2, y3, y4 };
    TransformBlitImage(object->image, x, y);
  }
end_method()

///////////////////////////////////////

begin_method(SS_IMAGE, ssImageTransformBlitMask, 9)
  if (This->m_ShouldRender) {
    arg_int(x1);
    arg_int(y1);
    arg_int(x2);
    arg_int(y2);
    arg_int(x3);
    arg_int(y3);
    arg_int(x4);
    arg_int(y4);
    arg_color(mask);

    int x[4] = { x1, x2, x3, x4 };
    int y[4] = { y1, y2, y3, y4 };
    TransformBlitImageMask(object->image, x, y, mask);
  }
end_method()

///////////////////////////////////////



///////////////////////////////////////
// SURFACE OBJECTS ////////////////////
///////////////////////////////////////

JSObject*
CScript::CreateSurfaceObject(JSContext* cx, CImage32* surface)
{
  // define surface class
  static JSClass clasp = {
    "surface", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeSurface,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  };
  
  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign the methods to the object
  static JSFunctionSpec fs[] = {
    { "blit",             ssSurfaceBlit,             2, 0, 0 },
    { "blitSurface",      ssSurfaceBlitSurface,      5, 0, 0 },
    { "createImage",      ssSurfaceCreateImage,      0, 0, 0 },
    { "setBlendMode",     ssSurfaceSetBlendMode,     1, 0, 0 },
    { "getPixel",         ssSurfaceGetPixel,         2, 0, 0 },
    { "setPixel",         ssSurfaceSetPixel,         3, 0, 0 },
    { "setAlpha",         ssSurfaceSetAlpha,         1, 0, 0 },
    { "line",             ssSurfaceLine,             5, 0, 0 },
    { "rectangle",        ssSurfaceRectangle,        5, 0, 0 },
    { "rotate",           ssSurfaceRotate,           2, 0, 0 },
    { "resize",           ssSurfaceResize,           2, 0, 0 },
    { "rescale",          ssSurfaceRescale,          2, 0, 0 },
    { "flipHorizontally", ssSurfaceFlipHorizontally, 0, 0, 0 },
    { "flipVertically",   ssSurfaceFlipVertically,   0, 0, 0 },
    { "clone",            ssSurfaceClone,            0, 0, 0 },
    { "cloneSection",     ssSurfaceCloneSection,     4, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // define width and height properties
  JS_DefineProperty(cx, object, "width",  INT_TO_JSVAL(surface->GetWidth()),  JS_PropertyStub, JS_PropertyStub, JSPROP_READONLY | JSPROP_PERMANENT);
  JS_DefineProperty(cx, object, "height", INT_TO_JSVAL(surface->GetHeight()), JS_PropertyStub, JS_PropertyStub, JSPROP_READONLY | JSPROP_PERMANENT);

  // attach the surface to this object
  SS_SURFACE* surface_object = new SS_SURFACE;
  surface_object->surface = surface;
  JS_SetPrivate(cx, object, surface_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_SURFACE, ssFinalizeSurface)
  delete object->surface;
end_finalizer()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceBlit, 2)
  if (This->m_ShouldRender) {

    arg_int(x);
    arg_int(y);
  
    DirectBlit(
      x,
      y,
      object->surface->GetWidth(),
      object->surface->GetHeight(),
      object->surface->GetPixels()
    );
  }
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceBlitSurface, 3)

  arg_surface(surface);
  arg_int(x);
  arg_int(y);

  if (surface) {
    object->surface->BlitImage(*surface, x, y);
  }

end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceCreateImage, 0)

  IMAGE image = CreateImage(
    object->surface->GetWidth(),
    object->surface->GetHeight(),
    object->surface->GetPixels()
  );

  if (image == NULL) {
    JS_ReportError(cx, "Fatal Error: CreateImage() failed");
    return JS_FALSE;
  }

  return_object(CreateImageObject(cx, image, true));
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceSetBlendMode, 1)
  arg_int(mode);
  object->surface->SetBlendMode(mode == CImage32::REPLACE ? CImage32::REPLACE : CImage32::BLEND);
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceGetPixel, 2)
  arg_int(x);
  arg_int(y);
  return_object(CreateColorObject(cx, object->surface->GetPixel(x, y)));
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceSetPixel, 3)
  arg_int(x);
  arg_int(y);
  arg_color(c);

  object->surface->SetPixel(x, y, c);
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceSetAlpha, 1)
  arg_int(alpha);

  object->surface->SetAlpha(alpha);
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceLine, 5)
  arg_int(x1);
  arg_int(y1);
  arg_int(x2);
  arg_int(y2);
  arg_color(c);

  object->surface->Line(x1, y1, x2, y2, c);
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceRectangle, 5)
  arg_int(x1);
  arg_int(y1);
  arg_int(x2);
  arg_int(y2);
  arg_color(c);

  object->surface->Rectangle(x1, y1, x2, y2, c);
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceRotate, 2)
  arg_int(degrees);
  arg_bool(autosize);

  object->surface->Rotate(degrees, autosize);
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceResize, 2)
  arg_int(w);
  arg_int(h);

  object->surface->Resize(w, h);
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceRescale, 2)
  arg_int(w);
  arg_int(h);

  object->surface->Rescale(w, h);
end_method();

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceFlipHorizontally, 0)
  object->surface->FlipHorizontal();
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceFlipVertically, 0)
  object->surface->FlipVertical();
end_method()

////////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceClone, 0)
  // create the surface
  CImage32* surface = new CImage32(*object->surface);
  return_object(CreateSurfaceObject(cx, surface));
end_method()  

///////////////////////////////////////

begin_method(SS_SURFACE, ssSurfaceCloneSection, 4)
  arg_int(x);
  arg_int(y);
  arg_int(w);
  arg_int(h);

  if (x < 0) {
    x = 0;
  }
  if (y < 0) {
    y = 0;
  }

  if (x + w > object->surface->GetWidth()) {
    w = object->surface->GetWidth() - x;
  }

  if (y + h > object->surface->GetHeight()) {
    h = object->surface->GetHeight() - y;
  }

  // create surface object
  CImage32* surface = new CImage32(w, h);

  for (int iy = 0; iy < h; iy++) {
    memcpy(
      surface->GetPixels() + iy * w,
      object->surface->GetPixels() + (iy + y) * object->surface->GetWidth() + x,
      w * sizeof(RGBA));
  }

  return_object(CreateSurfaceObject(cx, surface));
end_method()

///////////////////////////////////////



///////////////////////////////////////
// ANIMATION OBJECTS //////////////////
///////////////////////////////////////

JSObject*
CScript::CreateAnimationObject(JSContext* cx, IAnimation* animation)
{
  // define animation class
  static JSClass clasp = {
    "animation", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeAnimation,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  };
  
  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign the methods to the object
  static JSFunctionSpec fs[] = {
    { "getNumFrames",  ssAnimationGetNumFrames,  0, 0, 0 },
    { "getDelay",      ssAnimationGetDelay,      0, 0, 0 },
    { "readNextFrame", ssAnimationReadNextFrame, 0, 0, 0 },
    { "drawFrame",     ssAnimationDrawFrame,     2, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // define width and height properties
  JS_DefineProperty(cx, object, "width",  INT_TO_JSVAL(animation->GetWidth()),  JS_PropertyStub, JS_PropertyStub, JSPROP_READONLY | JSPROP_PERMANENT);
  JS_DefineProperty(cx, object, "height", INT_TO_JSVAL(animation->GetHeight()), JS_PropertyStub, JS_PropertyStub, JSPROP_READONLY | JSPROP_PERMANENT);

  // attach the animation to this object
  SS_ANIMATION* animation_object = new SS_ANIMATION;
  animation_object->animation = animation;
  animation_object->frame = new RGBA[animation->GetWidth() * animation->GetHeight()];
  animation->ReadNextFrame(animation_object->frame);
  JS_SetPrivate(cx, object, animation_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_ANIMATION, ssFinalizeAnimation)
  object->animation->Destroy();
  delete[] object->frame;
end_finalizer()

////////////////////////////////////////

begin_method(SS_ANIMATION, ssAnimationGetNumFrames, 0)
  return_int(object->animation->GetNumFrames());
end_method()

////////////////////////////////////////

begin_method(SS_ANIMATION, ssAnimationGetDelay, 0)
  return_int(object->animation->GetDelay());
end_method()

////////////////////////////////////////

begin_method(SS_ANIMATION, ssAnimationReadNextFrame, 0)
  object->animation->ReadNextFrame(object->frame);
end_method()

////////////////////////////////////////

begin_method(SS_ANIMATION, ssAnimationDrawFrame, 2)
  arg_int(x);
  arg_int(y);
  if (This->m_ShouldRender) {
    DirectBlit(x, y, object->animation->GetWidth(), object->animation->GetHeight(), object->frame);
  }
end_method()

///////////////////////////////////////


///////////////////////////////////////
// FILE OBJECTS ///////////////////////
///////////////////////////////////////

JSObject*
CScript::CreateFileObject(JSContext* cx, CConfigFile* file)
{
  // define file class
  static JSClass clasp = {
    "file", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeFile,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  };
  
  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // assign the methods to the object
  static JSFunctionSpec fs[] = {
    { "write", ssFileWrite, 2, 0, 0 },
    { "read",  ssFileRead,  2, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // attach the file to this object
  SS_FILE* file_object = new SS_FILE;
  file_object->file = file;
  JS_SetPrivate(cx, object, file_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_FILE, ssFinalizeFile)
  This->m_Engine->CloseFile(object->file);
end_finalizer()

////////////////////////////////////////

begin_method(SS_FILE, ssFileWrite, 2)
  arg_str(key);

  if (JSVAL_IS_INT(argv[1])) {
    object->file->WriteInt("", key, JSVAL_TO_INT(argv[1]));
  } else if (JSVAL_IS_BOOLEAN(argv[1])) {
    object->file->WriteBool("", key, (JSVAL_TO_BOOLEAN(argv[1]) == JS_TRUE));
  } else if (JSVAL_IS_DOUBLE(argv[1])) {
    double* d = JSVAL_TO_DOUBLE(argv[1]);
    object->file->WriteDouble("", key, *d);
  } else { // anything else is a string
    object->file->WriteString("", key, argStr(cx, argv[1]));
  }
end_method()

////////////////////////////////////////

begin_method(SS_FILE, ssFileRead, 2)
  arg_str(key);

  if (JSVAL_IS_INT(argv[1])) {
    int i = object->file->ReadInt("", key, JSVAL_TO_INT(argv[1]));
    return_int(i);
  } else if (JSVAL_IS_BOOLEAN(argv[1])) {
    bool b = object->file->ReadBool("", key, (JSVAL_TO_BOOLEAN(argv[1]) == JS_TRUE));
    return_bool(b);
  } else if (JSVAL_IS_DOUBLE(argv[1])) {
    double* def = JSVAL_TO_DOUBLE(argv[1]);
    double d = object->file->ReadDouble("", key, *def);
    return_double(d);
  } else { // anything else is a string
    std::string str = object->file->ReadString("", key, argStr(cx, argv[1]));
    return_str(str.c_str());
  }
end_method()  

///////////////////////////////////////


///////////////////////////////////////
// RAW FILE OBJECTS ///////////////////
///////////////////////////////////////

JSObject*
CScript::CreateRawFileObject(JSContext* cx, IFile* file)
{
  // define raw file class
  static JSClass clasp = {
    "rawfile", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeRawFile,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  };

  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // add the methods into the object
  static JSFunctionSpec fs[] = {
    { "setPosition", ssRawFileSetPosition, 1, 0, 0 },
    { "getPosition", ssRawFileGetPosition, 0, 0, 0 },
    { "getSize",     ssRawFileGetSize,     0, 0, 0 },
    { "read",        ssRawFileRead,        1, 0, 0 },
    { 0, 0, 0, 0, 0 },
  };
  JS_DefineFunctions(cx, object, fs);

  // attach the file to this object
  SS_RAWFILE* file_object = new SS_RAWFILE;
  file_object->file = file;
  JS_SetPrivate(cx, object, file_object);

  return object;
}

////////////////////////////////////////

begin_finalizer(SS_RAWFILE, ssFinalizeRawFile)
  object->file->Close();
end_finalizer()

////////////////////////////////////////

begin_method(SS_RAWFILE, ssRawFileSetPosition, 1)
  arg_int(position);
  object->file->Seek(position);
end_method()

////////////////////////////////////////

begin_method(SS_RAWFILE, ssRawFileGetPosition, 0)
  return_int(object->file->Tell());
end_method()

////////////////////////////////////////

begin_method(SS_RAWFILE, ssRawFileGetSize, 0)
  return_int(object->file->Size());
end_method()

////////////////////////////////////////

begin_method(SS_RAWFILE, ssRawFileRead, 1)
  arg_int(size);

  // read the data
  byte* data = new byte[size];
  int bytes_read = object->file->Read(data, size);

  // convert data, and turn it into an object :)
  jsval* converted_data = new jsval[bytes_read];
  for (int i = 0; i < bytes_read; i++)
    converted_data[i] = INT_TO_JSVAL(data[i]);
  return_object(JS_NewArrayObject(cx, bytes_read, converted_data));

  delete[] data;
  delete[] converted_data;
end_method()

///////////////////////////////////////


///////////////////////////////////////
// BYTE ARRAY OBJECTS /////////////////
///////////////////////////////////////

JSObject*
CScript::CreateByteArrayObject(JSContext* cx, int size, const void* data)
{
  // define file class
  static JSClass clasp = {
    "byte_array", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, ssByteArrayGetProperty, ssByteArraySetProperty,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ssFinalizeByteArray,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  };
  
  // create the object
  JSObject* object = JS_NewObject(cx, &clasp, NULL, NULL);
  if (object == NULL) {
    return NULL;
  }

  // give the object a "length" property
  JS_DefineProperty(cx, object, "length", INT_TO_JSVAL(size), JS_PropertyStub, JS_PropertyStub, JSPROP_READONLY | JSPROP_PERMANENT);

  // attach the file to this object
  SS_BYTEARRAY* array_object = new SS_BYTEARRAY;
  array_object->size = size;
  array_object->array = new byte[size];

  if (data) {
    memcpy(array_object->array, data, size);
  } else {
    memset(array_object->array, 0, size);
  }

  JS_SetPrivate(cx, object, array_object);

  return object;
}

///////////////////////////////////////

begin_finalizer(SS_BYTEARRAY, ssFinalizeByteArray)
  delete[] object->array;
end_finalizer()

///////////////////////////////////////

begin_property(SS_BYTEARRAY, ssByteArrayGetProperty)
  int prop_id = argInt(cx, id);
  if (prop_id < 0 || prop_id >= object->size) {
    JS_ReportError(cx, "Byte array access out of bounds (%d)", prop_id);
    return JS_FALSE;
  }

  *vp = INT_TO_JSVAL(object->array[prop_id]);
end_property()

////////////////////////////////////////

begin_property(SS_BYTEARRAY, ssByteArraySetProperty)
  int prop_id = argInt(cx, id);
  if (prop_id < 0 || prop_id >= object->size) {
    JS_ReportError(cx, "Byte array access out of bounds (%d)", prop_id);
    return JS_FALSE;
  }

  object->array[prop_id] = (byte)argInt(cx, *vp);
end_property()

////////////////////////////////////////