#ifndef VM_INTERFACE_HPP
#define VM_INTERFACE_HPP


#include "audio.hpp"
#include "sfont.hpp"
#include "sspriteset.hpp"
#include "swindowstyle.hpp"
#include "log.hpp"
#include "sphere.hpp"
#include "../common/configfile.hpp"
#include "../common/IAnimation.hpp"
#include "../common/IFileSystem.hpp"


class CMapEngine;


struct IEngine
{
  virtual ~IEngine() { }

  // types
  typedef void* script;

  // methods
  virtual bool IsScriptEvaluated(const char* filename);
  virtual bool IsSystemScriptEvaluated(const char* filename);
  virtual void AddEvaluatedScript(const char* filename);
  virtual void AddEvaluatedSystemScript(const char* filename);
  
  virtual bool GetScriptText(const char* filename, std::string& text) = 0;
  virtual bool GetSystemScript(const char* filename, std::string& text) = 0;

  virtual script CompileScript(const char* script, std::string& error) = 0;
  virtual bool ExecuteScript(script s, bool& should_exit, std::string& error) = 0;
  virtual void DestroyScript(script s) = 0;

  virtual void GetGameList(std::vector<Game>& games) = 0;
  virtual void ExecuteGame(const char* directory) = 0;

  virtual CMapEngine* GetMapEngine() = 0;

  virtual CLog* OpenLog(const char* filename) = 0;
  virtual void CloseLog(CLog* log) = 0;

  virtual SSPRITESET* LoadSpriteset(const char* filename) = 0;
  virtual void DestroySpriteset(SSPRITESET* spriteset) = 0;

  virtual SFONT* GetSystemFont() = 0;
  virtual SFONT* LoadFont(const char* filename) = 0;
  virtual void DestroyFont(SFONT* font) = 0;

  virtual SWINDOWSTYLE* GetSystemWindowStyle() = 0;
  virtual SWINDOWSTYLE* LoadWindowStyle(const char* filename) = 0;
  virtual void DestroyWindowStyle(SWINDOWSTYLE* font) = 0;

  virtual audiere::OutputStream* LoadSound(const char* filename, bool streaming) = 0;

  virtual IMAGE GetSystemArrow() = 0;
  virtual IMAGE GetSystemUpArrow() = 0;
  virtual IMAGE GetSystemDownArrow() = 0;
  virtual IMAGE LoadImage(const char* filename) = 0;
  virtual void DestroyImage(IMAGE image) = 0;

  virtual CImage32* LoadSurface(const char* filename) = 0;
  virtual void DestroySurface(CImage32* surface) = 0;

  virtual IAnimation* LoadAnimation(const char* filename) = 0;

  virtual void GetFileList(const char* directory, std::vector<std::string>& vs) = 0;
  virtual CConfigFile* OpenFile(const char* filename) = 0;
  virtual void CloseFile(CConfigFile* file) = 0;

  virtual IFile* OpenRawFile(const char* filename, bool writeable) = 0;

  std::vector<std::string> m_EvaluatedScripts;
  std::vector<std::string> m_EvaluatedSystemScripts;
};


#endif
