#ifndef ENGINE_HPP
#define ENGINE_HPP


// identifier too long
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "engineinterface.hpp"
#include "script.hpp"
#include "sphere.hpp"
#include "../common/IFileSystem.hpp"


class CGameEngine : private IEngine
{
public:
  CGameEngine(
    IFileSystem& fs,
    const SSystemObjects& system_objects,
    const std::vector<Game>& game_list,
    const char* script_directory,
    const char* parameters);

  /**
   * Runs the game.  If this returns a value, run a new game and then
   * run this game again (push the new game on a disk).  Otherwise, do
   * nothing.
   */
  std::string Run();

private:
  void ShowError(const char* message);
  void CreateSystemObjects();
  void DestroySystemObjects();

  // IEngine implementation

  virtual bool GetScriptText(const char* filename, std::string& text);
  virtual bool GetSystemScript(const char* filename, std::string& text);

  virtual void GetGameList(std::vector<Game>& games);
  virtual void ExecuteGame(const char* directory);

  virtual script CompileScript(const char* script, std::string& error);
  virtual bool ExecuteScript(script s, bool& should_exit, std::string& error);
  virtual void DestroyScript(script s);

  virtual CMapEngine* GetMapEngine();

  virtual CLog* OpenLog(const char* filename);
  virtual void CloseLog(CLog* log);

  virtual SSPRITESET* LoadSpriteset(const char* filename);
  virtual void DestroySpriteset(SSPRITESET* spriteset);
  
  virtual SFONT* GetSystemFont();
  virtual SFONT* LoadFont(const char* filename);
  virtual void DestroyFont(SFONT* font);

  virtual SWINDOWSTYLE* GetSystemWindowStyle();
  virtual SWINDOWSTYLE* LoadWindowStyle(const char* filename);
  virtual void DestroyWindowStyle(SWINDOWSTYLE* ws);

  virtual audiere::OutputStream* LoadSound(const char* filename, bool streaming);

  virtual IMAGE GetSystemArrow();
  virtual IMAGE GetSystemUpArrow();
  virtual IMAGE GetSystemDownArrow();
  virtual IMAGE LoadImage(const char* filename);
  virtual void DestroyImage(IMAGE image);

  virtual CImage32* LoadSurface(const char* filename);
  virtual void DestroySurface(CImage32* surface);

  virtual IAnimation* LoadAnimation(const char* filename);

  virtual void GetFileList(const char* directory, std::vector<std::string>& vs);
  virtual CConfigFile* OpenFile(const char* filename);
  virtual void CloseFile(CConfigFile* file);
  
  virtual IFile* OpenRawFile(const char* filename);

private:
  struct SFileInfo {
    std::string filename;
    int         refcount;
  };

private:
  IFileSystem& m_FileSystem;
  const SSystemObjects& m_SystemObjects;
  const std::vector<Game>& m_GameList;
  std::string m_SystemScriptDirectory;
  const char* m_Parameters;

  // loaded system objects
  SFONT        m_SystemFont;
  SWINDOWSTYLE m_SystemWindowStyle;
  IMAGE        m_SystemArrow;
  IMAGE        m_SystemUpArrow;
  IMAGE        m_SystemDownArrow;

  CScript* m_Script;

  // the next game to be executed
  std::string m_NextGame;

  // opened file list
  std::map<CConfigFile*, SFileInfo> m_OpenFiles;

  CMapEngine* m_MapEngine;
};


#endif
