#ifndef SCRIPT_HPP
#define SCRIPT_HPP


// identifier too long
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <string>
#include <jsapi.h>
#include "audio.hpp"
#include "engineinterface.hpp"
#include "sfont.hpp"
#include "sspriteset.hpp"
#include "swindowstyle.hpp"
#include "video.hpp"
#include "network.hpp"
#include "../common/ConfigFile.hpp"
#include "../common/IAnimation.hpp"
#include "../common/IFileSystem.hpp"
#include "../common/Image32.hpp"


// EVIL EVIL export!
struct SS_SPRITESET;


class CScript; // forward declaration for CScriptCode


// encapsulates a snippet of code
// (already compiled in JS)
class CScriptCode
{
public:
  void AddRef();
  void Release();
  bool Execute(bool& should_exit); // return value of the script is ignored

private:
  CScriptCode(JSContext* context, JSObject* global, JSScript* script);
  ~CScriptCode();

private:
  JSContext* m_Context;
  JSObject*  m_Global;
  JSScript*  m_Script;
  JSObject*  m_ScriptObject;
  int        m_NumReferences;

  friend CScript;
};


class CScript
{
public:
  CScript(IEngine* engine);
  ~CScript();

  // fails if could not load, compile, or run
  bool EvaluateFile(const char* filename, IFileSystem& fs);
  bool Evaluate(const char* script, const char* filename = NULL);
  CScriptCode* Compile(const char* script);

  // valid if Evaluate() or EvaluateFile() return false
  // also if a script code object fails on Execute()
  const char* GetError();

private:
  void InitializeSphereFunctions();
  void InitializeSphereConstants();
  void ReportMapEngineError(const char* header);

  static void   ErrorReporter(JSContext* cx, const char* message, JSErrorReport* report);
  static JSBool BranchCallback(JSContext* cx, JSScript* script);

  // global functions
  
  #define SS_FUNCTION(name, numargs) static JSBool ss##name(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval);
  #include "ss_functions.table"
  #undef SS_FUNCTION


  // objects

  #define declare_constructor1(name, param1)                 \
    static JSObject* name(JSContext* cx, param1);
  #define declare_constructor2(name, param1, param2)         \
    static JSObject* name(JSContext* cx, param1, param2);
  #define declare_finalizer(name) static void name(JSContext* cx, JSObject* obj)
  #define declare_method(name)    static JSBool name(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  #define declare_property(name)  static JSBool name(JSContext* cx, JSObject* obj, jsval id, jsval* vp)


  // sockets
  declare_constructor1(CreateSocketObject, NSOCKET socket);
  declare_finalizer(ssFinalizeSocket);
  declare_method(ssSocketIsConnected);
  declare_method(ssSocketGetPendingReadSize);
  declare_method(ssSocketWrite);
  declare_method(ssSocketRead);

  // logs
  declare_constructor1(CreateLogObject, CLog* log);
  declare_finalizer(ssFinalizeLog);
  declare_method(ssLogWrite);
  declare_method(ssLogBeginBlock);
  declare_method(ssLogEndBlock);

  // colors
  declare_constructor1(CreateColorObject, RGBA color);
  declare_finalizer(ssFinalizeColor);
  declare_property(ssColorGetProperty);
  declare_property(ssColorSetProperty);

  // spriteset
  declare_constructor1(CreateSpritesetObject, SSPRITESET* spriteset);
  declare_finalizer(ssFinalizeSpriteset);

  // sounds
  declare_constructor1(CreateSoundObject, ADR_STREAM sound);
  declare_finalizer(ssFinalizeSound);
  declare_method(ssSoundPlay);
  declare_method(ssSoundStop);
  declare_method(ssSoundSetVolume);
  declare_method(ssSoundGetVolume);
  declare_method(ssSoundSetPan);
  declare_method(ssSoundGetPan);
  declare_method(ssSoundIsPlaying);

  // fonts
  declare_constructor2(CreateFontObject, SFONT* font, bool destroy);
  declare_finalizer(ssFinalizeFont);
  declare_method(ssFontSetColorMask);
  declare_method(ssFontDrawText);
  declare_method(ssFontDrawZoomedText);
  declare_method(ssFontDrawTextBox);
  declare_method(ssFontGetHeight);
  declare_method(ssFontGetStringWidth);

  // window styles
  declare_constructor2(CreateWindowStyleObject, SWINDOWSTYLE* ws, bool destroy);
  declare_finalizer(ssFinalizeWindowStyle);
  declare_method(ssWindowStyleDrawWindow);

  // images
  declare_constructor2(CreateImageObject, IMAGE image, bool destroy);
  declare_finalizer(ssFinalizeImage);
  declare_method(ssImageBlit);
  declare_method(ssImageBlitMask);
  declare_method(ssImageRotateBlit);
  declare_method(ssImageZoomBlit);
  declare_method(ssImageTransformBlit);
  declare_method(ssImageTransformBlitMask);

  // surfaces
  declare_constructor1(CreateSurfaceObject, CImage32* surface);
  declare_finalizer(ssFinalizeSurface);
  declare_method(ssSurfaceBlit);
  declare_method(ssSurfaceBlitSurface);
  declare_method(ssSurfaceCreateImage);
  declare_method(ssSurfaceSetBlendMode);
  declare_method(ssSurfaceGetPixel);
  declare_method(ssSurfaceSetPixel);
  declare_method(ssSurfaceSetAlpha);
  declare_method(ssSurfaceLine);
  declare_method(ssSurfaceRectangle);
  declare_method(ssSurfaceRotate);
  declare_method(ssSurfaceResize);
  declare_method(ssSurfaceRescale);
  declare_method(ssSurfaceFlipHorizontally);
  declare_method(ssSurfaceFlipVertically);
  declare_method(ssSurfaceClone);
  declare_method(ssSurfaceCloneSection);

  // animations
  declare_constructor1(CreateAnimationObject, IAnimation* animation);
  declare_finalizer(ssFinalizeAnimation);
  declare_method(ssAnimationGetNumFrames);
  declare_method(ssAnimationGetDelay);
  declare_method(ssAnimationReadNextFrame);
  declare_method(ssAnimationDrawFrame);

  // files
  declare_constructor1(CreateFileObject, CConfigFile* file);
  declare_finalizer(ssFinalizeFile);
  declare_method(ssFileRead);
  declare_method(ssFileWrite);

  // raw files
  declare_constructor1(CreateRawFileObject, IFile* file);
  declare_finalizer(ssFinalizeRawFile);
  declare_method(ssRawFileSetPosition);
  declare_method(ssRawFileGetPosition);
  declare_method(ssRawFileGetSize);
  declare_method(ssRawFileRead);

  // byte arrays
  declare_constructor2(CreateByteArrayObject, int size, const void* data = NULL);
  declare_finalizer(ssFinalizeByteArray);
  declare_property(ssByteArrayGetProperty);
  declare_property(ssByteArraySetProperty);

  
  #undef declare_property
  #undef declare_method
  #undef declare_finalizer

  
private:
  IEngine* m_Engine;

  JSRuntime* m_Runtime;
  JSContext* m_Context;
  JSObject*  m_Global;

  std::string m_Error;  // if empty, there is no error
  bool m_ShouldExit;

  int m_GCCount;


  // used in native functions
  int m_RecurseCount;  // for RunScript()

  bool m_ShouldRender; // graphics functions
  int m_FrameRate;     // .
  int m_FramesSkipped; // .
  int m_IdealTime;     // .


  // it needs to access m_Error
  friend CScriptCode;
};


#endif