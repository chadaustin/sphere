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
#include "../common/configfile.hpp"
#include "../common/IAnimation.hpp"
#include "../common/IFileSystem.hpp"
#include "../common/Image32.hpp"
#include "../common/ColorMatrix.hpp"
#include "../common/Map.hpp"
#include "../common/Layer.hpp"


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
  bool IsBeingUsed();

private:
  CScriptCode(JSContext* context, JSObject* global, JSScript* script);
  ~CScriptCode();

private:
  JSContext* m_Context;
  JSObject*  m_Global;
  JSScript*  m_Script;
  JSObject*  m_ScriptObject;
  int        m_NumReferences;
  bool       m_BeingUsed;

  friend class CScript;
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
  declare_method(ssSocketClose);

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
  declare_method(ssSpritesetSave);
  declare_method(ssSpritesetClone);

  // spriteset base
  declare_constructor1(CreateSpritesetBaseObject, SSPRITESET* spriteset);

  // sounds
  declare_constructor1(CreateSoundObject, audiere::OutputStream* sound);
  declare_finalizer(ssFinalizeSound);
  declare_method(ssSoundPlay);
  declare_method(ssSoundPause);
  declare_method(ssSoundStop);
  declare_method(ssSoundSetVolume);
  declare_method(ssSoundGetVolume);
  declare_method(ssSoundSetPan);
  declare_method(ssSoundGetPan);
  declare_method(ssSoundSetPitch);
  declare_method(ssSoundGetPitch);
  declare_method(ssSoundIsPlaying);
  declare_method(ssSoundClone);
  declare_method(ssSoundGetPosition);
  declare_method(ssSoundSetPosition);
  declare_method(ssSoundIsSeekable);
  declare_method(ssSoundGetLength);

  // fonts
  declare_constructor2(CreateFontObject, SFONT* font, bool destroy);
  declare_finalizer(ssFinalizeFont);
  declare_method(ssFontSetColorMask);
  declare_method(ssFontGetColorMask);
  declare_method(ssFontDrawText);
  declare_method(ssFontDrawZoomedText);
  declare_method(ssFontDrawTextBox);
  declare_method(ssFontGetHeight);
  declare_method(ssFontGetStringWidth);
  declare_method(ssFontGetStringHeight);
  declare_method(ssFontClone);
  declare_method(ssFontSave);
  declare_method(ssFontGetCharacterImage);
  declare_method(ssFontSetCharacterImage);

  // window styles
  declare_constructor2(CreateWindowStyleObject, SWINDOWSTYLE* ws, bool destroy);
  declare_finalizer(ssFinalizeWindowStyle);
  declare_method(ssWindowStyleDrawWindow);
  declare_method(ssWindowStyleSetColorMask);
  declare_method(ssWindowStyleGetColorMask);

  // images
  declare_constructor2(CreateImageObject, IMAGE image, bool destroy);
  declare_finalizer(ssFinalizeImage);
  declare_method(ssImageBlit);
  declare_method(ssImageBlitMask);
  declare_method(ssImageRotateBlit);
  declare_method(ssImageRotateBlitMask);
  declare_method(ssImageZoomBlit);
  declare_method(ssImageZoomBlitMask);
  declare_method(ssImageTransformBlit);
  declare_method(ssImageTransformBlitMask);
  declare_method(ssImageCreateSurface);

  // surfaces
  declare_constructor1(CreateSurfaceObject, CImage32* surface);
  declare_finalizer(ssFinalizeSurface);
  declare_method(ssSurfaceApplyColorFX);
  declare_method(ssSurfaceApplyColorFX4);
  declare_method(ssSurfaceBlit);
  declare_method(ssSurfaceBlitSurface);
  //declare_method(ssSurfaceBlitImage);
  declare_method(ssSurfaceCreateImage);
  declare_method(ssSurfaceSetBlendMode);
  declare_method(ssSurfaceGetPixel);
  declare_method(ssSurfaceSetPixel);
  declare_method(ssSurfaceSetAlpha);
  declare_method(ssSurfaceReplaceColor);
  declare_method(ssSurfaceLine);
  declare_method(ssSurfaceRectangle);
  declare_method(ssSurfaceTriangle);
  declare_method(ssSurfaceGradientLine);
  declare_method(ssSurfaceGradientRectangle);
  declare_method(ssSurfaceGradientTriangle);
  declare_method(ssSurfaceRotate);
  declare_method(ssSurfaceResize);
  declare_method(ssSurfaceRescale);
  declare_method(ssSurfaceFlipHorizontally);
  declare_method(ssSurfaceFlipVertically);
  declare_method(ssSurfaceClone);
  declare_method(ssSurfaceCloneSection);
  declare_method(ssSurfaceDrawText);
  declare_method(ssSurfaceDrawZoomedText);
  declare_method(ssSurfaceDrawTextBox);
  declare_method(ssSurfaceSave);
  declare_method(ssSurfaceApplyLookup);

  // color matrix
  declare_constructor1(CreateColorMatrixObject, CColorMatrix* colormatrix);
  declare_finalizer(ssFinalizeColorMatrix);

  // animations
  declare_constructor1(CreateAnimationObject, IAnimation* animation);
  declare_finalizer(ssFinalizeAnimation);
  declare_method(ssAnimationGetNumFrames);
  declare_method(ssAnimationGetDelay);
  declare_method(ssAnimationReadNextFrame);
  declare_method(ssAnimationDrawFrame);
  declare_method(ssAnimationDrawZoomedFrame);

  // files
  declare_constructor1(CreateFileObject, CConfigFile* file);
  declare_finalizer(ssFinalizeFile);
  declare_method(ssFileRead);
  declare_method(ssFileWrite);
  declare_method(ssFileFlush);
  declare_method(ssFileClose);

  // raw files
  declare_constructor2(CreateRawFileObject, IFile* file, bool writeable);
  declare_finalizer(ssFinalizeRawFile);
  declare_method(ssRawFileSetPosition);
  declare_method(ssRawFileGetPosition);
  declare_method(ssRawFileGetSize);
  declare_method(ssRawFileRead);
  declare_method(ssRawFileWrite);
  declare_method(ssRawFileClose);

  // byte arrays
  declare_constructor2(CreateByteArrayObject, int size, const void* data = NULL);
  declare_finalizer(ssFinalizeByteArray);
  declare_property(ssByteArrayGetProperty);
  declare_property(ssByteArraySetProperty);
  declare_method(ssByteArrayConcat);
  declare_method(ssByteArraySlice);

  // mapengine objects
  declare_method(ssMapEngineSave);
  declare_method(ssMapEngineLayerAppend);

  // tileset objects
  declare_constructor1(CreateTilesetObject, const sTileset& tileset);
  declare_method(ssTilesetAppendTiles);
  declare_method(ssTilesetSave);
  
  #undef declare_property
  #undef declare_method
  #undef declare_finalizer

  
private:
  // should we draw graphics?
  bool ShouldRender() {
    return m_ShouldRender;
  }

  IEngine* m_Engine;

  JSRuntime* m_Runtime;
  JSContext* m_Context;
  JSObject*  m_Global;

  std::string m_Error;  // if empty, there is no error
  bool m_ShouldExit;

  bool m_GCEnabled;
  int m_GCCount;


  // used in native functions
  int m_RecurseCount;  // for RunScript()

  bool m_ShouldRender; // graphics functions
  int m_FrameRate;     // .
  int m_FramesSkipped; // .
  int m_IdealTime;     // .

  // it needs to access m_Error
  friend class CScriptCode;
  
  // it needs to access m_GCEnabled
  friend class NoGCBlock;
};


#endif
