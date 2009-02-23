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

#include "../particle_engine/ParticleSystemParent.hpp"
#include "../particle_engine/ParticleSystemChild.hpp"



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


  // particle body
  declare_constructor1(CreateParticleBodyObject, ParticleSystemBase* system);
  declare_finalizer(ssFinalizeParticleBody);
  declare_property(ssParticleBodyGetProperty);
  declare_property(ssParticleBodySetProperty);

  // particle initializer
  declare_constructor1(CreateParticleInitializerObject, ParticleSystemBase* system);
  declare_finalizer(ssFinalizeParticleInitializer);
  declare_property(ssParticleInitializerGetProperty);
  declare_property(ssParticleInitializerSetProperty);

  // particle updater
  declare_constructor1(CreateParticleUpdaterObject, ParticleSystemBase* system);
  declare_finalizer(ssFinalizeParticleUpdater);
  declare_property(ssParticleUpdaterGetProperty);
  declare_property(ssParticleUpdaterSetProperty);

  // particle renderer
  declare_constructor1(CreateParticleRendererObject, ParticleSystemBase* system);
  declare_finalizer(ssFinalizeParticleRenderer);
  declare_property(ssParticleRendererGetProperty);
  declare_property(ssParticleRendererSetProperty);

  // particle callback
  declare_constructor1(CreateParticleCallbackObject, ParticleSystemBase* system);
  declare_finalizer(ssFinalizeParticleCallback);
  declare_property(ssParticleCallbackGetProperty);
  declare_property(ssParticleCallbackSetProperty);

  // particle system parent
  declare_constructor1(CreateParticleSystemParentObject, ParticleSystemParent* system);
  declare_finalizer(ssFinalizeParticleSystemParent);
  declare_property(ssParticleSystemParentGetProperty);
  declare_property(ssParticleSystemParentSetProperty);
  declare_method(ssParticleSystemParentUpdate);
  declare_method(ssParticleSystemParentRender);
  declare_method(ssParticleSystemParentSize);
  declare_method(ssParticleSystemParentClear);
  declare_method(ssParticleSystemParentAdopt);
  declare_method(ssParticleSystemParentHost);
  declare_method(ssParticleSystemParentFind);
  declare_method(ssParticleSystemParentRemove);
  declare_method(ssParticleSystemParentRemoveGroup);
  declare_method(ssParticleSystemParentIsDead);
  declare_method(ssParticleSystemParentKill);
  declare_method(ssParticleSystemParentRevive);
  declare_method(ssParticleSystemParentGetOnUpdate);
  declare_method(ssParticleSystemParentSetOnUpdate);
  declare_method(ssParticleSystemParentGetOnRender);
  declare_method(ssParticleSystemParentSetOnRender);
  declare_method(ssParticleSystemParentGetOnBirth);
  declare_method(ssParticleSystemParentSetOnBirth);
  declare_method(ssParticleSystemParentGetOnDeath);
  declare_method(ssParticleSystemParentSetOnDeath);

  // particle swarm renderer
  declare_constructor1(CreateParticleSwarmRendererObject, ParticleSystemChild* system);
  declare_finalizer(ssFinalizeParticleSwarmRenderer);
  declare_property(ssParticleSwarmRendererGetProperty);
  declare_property(ssParticleSwarmRendererSetProperty);

  // particle system child
  declare_constructor1(CreateParticleSystemChildObject, ParticleSystemChild* system);
  declare_finalizer(ssFinalizeParticleSystemChild);
  declare_property(ssParticleSystemChildGetProperty);
  declare_property(ssParticleSystemChildSetProperty);
  declare_method(ssParticleSystemChildUpdate);
  declare_method(ssParticleSystemChildRender);
  declare_method(ssParticleSystemChildSize);
  declare_method(ssParticleSystemChildCapacity);
  declare_method(ssParticleSystemChildGrow);
  declare_method(ssParticleSystemChildShrink);
  declare_method(ssParticleSystemChildResize);
  declare_method(ssParticleSystemChildReserve);
  declare_method(ssParticleSystemChildClear);
  declare_method(ssParticleSystemChildClone);
  declare_method(ssParticleSystemChildIsDead);
  declare_method(ssParticleSystemChildKill);
  declare_method(ssParticleSystemChildRevive);
  declare_method(ssParticleSystemChildGetOnUpdate);
  declare_method(ssParticleSystemChildSetOnUpdate);
  declare_method(ssParticleSystemChildGetOnRender);
  declare_method(ssParticleSystemChildSetOnRender);
  declare_method(ssParticleSystemChildGetOnBirth);
  declare_method(ssParticleSystemChildSetOnBirth);
  declare_method(ssParticleSystemChildGetOnDeath);
  declare_method(ssParticleSystemChildSetOnDeath);


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
#if defined(WIN32) && defined(USE_MIDI)
  declare_constructor2(CreateSoundObject, audiere::OutputStream* sound, audiere::MIDIStream* midi);
#else
  declare_constructor1(CreateSoundObject, audiere::OutputStream* sound);
#endif
  declare_finalizer(ssFinalizeSound);
  declare_method(ssSoundPlay);
  declare_method(ssSoundPause);
  declare_method(ssSoundStop);
  declare_method(ssSoundReset);
  declare_method(ssSoundSetRepeat);
  declare_method(ssSoundGetRepeat);
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

  // sound effects
  declare_constructor1(CreateSoundEffectObject, audiere::SoundEffect* sound);
  declare_finalizer(ssFinalizeSoundEffect);
  declare_method(ssSoundEffectPlay);
  declare_method(ssSoundEffectStop);
  declare_method(ssSoundEffectSetVolume);
  declare_method(ssSoundEffectGetVolume);
  declare_method(ssSoundEffectSetPan);
  declare_method(ssSoundEffectGetPan);
  declare_method(ssSoundEffectSetPitch);
  declare_method(ssSoundEffectGetPitch);

  // fonts
  declare_constructor2(CreateFontObject, SFONT* font, bool destroy);
  declare_finalizer(ssFinalizeFont);
  declare_method(ssFontSetColorMask);
  declare_method(ssFontGetColorMask);
  declare_method(ssFontDrawText);
  declare_method(ssFontDrawZoomedText);
  declare_method(ssFontDrawTextBox);
  declare_method(ssFontWordWrapString);
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
  declare_method(ssWindowStyleSave);
  declare_method(ssWindowStyleClone);

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
  declare_method(ssImageClone);

  // surfaces
  declare_constructor1(CreateSurfaceObject, CImage32* surface);
  declare_finalizer(ssFinalizeSurface);
  declare_method(ssSurfaceApplyColorFX);
  declare_method(ssSurfaceApplyColorFX4);
  declare_method(ssSurfaceBlit);
  declare_method(ssSurfaceBlitSurface);
  declare_method(ssSurfaceBlitMaskSurface);
  declare_method(ssSurfaceRotateBlitSurface);
  declare_method(ssSurfaceRotateBlitMaskSurface);
  declare_method(ssSurfaceZoomBlitSurface);
  declare_method(ssSurfaceZoomBlitMaskSurface);
  declare_method(ssSurfaceTransformBlitSurface);
  declare_method(ssSurfaceTransformBlitMaskSurface);
  //declare_method(ssSurfaceBlitImage);
  declare_method(ssSurfaceCreateImage);
  declare_method(ssSurfaceSetBlendMode);
  declare_method(ssSurfaceGetPixel);
  declare_method(ssSurfaceSetPixel);
  declare_method(ssSurfaceSetAlpha);
  declare_method(ssSurfaceReplaceColor);
  declare_method(ssSurfacePointSeries);
  declare_method(ssSurfaceLine);
  declare_method(ssSurfaceGradientLine);
  declare_method(ssSurfaceLineSeries);
  declare_method(ssSurfaceBezierCurve);
  declare_method(ssSurfaceOutlinedRectangle);
  declare_method(ssSurfaceRectangle);
  declare_method(ssSurfaceGradientRectangle);
  declare_method(ssSurfaceTriangle);
  declare_method(ssSurfaceGradientTriangle);
  declare_method(ssSurfacePolygon);
  declare_method(ssSurfaceOutlinedEllipse);
  declare_method(ssSurfaceFilledEllipse);
  declare_method(ssSurfaceOutlinedCircle);
  declare_method(ssSurfaceFilledCircle);
  declare_method(ssSurfaceGradientCircle);
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
  declare_method(ssFileGetNumKeys);
  declare_method(ssFileGetKey);

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

#ifdef _3D_FUNCTIONS
  //3d functions
  declare_method(ssImageTransform3DBlit);
  declare_method(ssImageTriangle3DBlit);
#endif // _3D_FUNCTIONS

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
