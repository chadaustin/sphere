#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <algorithm>
#include <memory>
#include "engine.hpp"
#include "map_engine.hpp"
#include "filesystem.hpp"
#include "inputx.hpp"
#include "render.hpp"
#include "system.hpp"
#include "time.hpp"
#include "../common/AnimationFactory.hpp"
#include "../common/configfile.hpp"
#include "../common/sphere_version.h"

////////////////////////////////////////////////////////////////////////////////

CGameEngine::CGameEngine(IFileSystem& fs,
                         const SSystemObjects& system_objects,
                         const std::vector<Game>& game_list,
                         const char* script_directory,
                         const char* parameters)
: m_FileSystem(fs)
, m_SystemObjects(system_objects)
, m_GameList(game_list)
, m_SystemScriptDirectory(script_directory)
, m_Parameters(parameters)
, m_Script(NULL)
, m_RestartGame(false)
, m_MapEngine(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

std::string
CGameEngine::Run()
{
  m_Script = new CScript(this);
  if (!m_Script)
    return "";

  m_RestartGame = false;

  // initialize map engine
  m_MapEngine = new CMapEngine(this, m_FileSystem);

  // initialize - load game information
  CConfigFile game_information;
  if ( !game_information.Load("game.sgm", m_FileSystem) ) {
  //  ShowError("Unable to load game.sgm");
  }

  int game_width  = game_information.ReadInt("", "screen_width",  320);
  int game_height = game_information.ReadInt("", "screen_height", 240);

  if (game_width <= 0) game_width = 320;
  if (game_height <= 0) game_height = 240;

  SwitchResolution(game_width, game_height);
  ClearScreen();

  // set the game title
  if (true) {
    char caption[125];
    const std::string& name = game_information.ReadString("", "name", "");

    if (name.empty()) {
      sprintf(caption, "Sphere %s", SPHERE_VERSION);
    }
    else {
      sprintf(caption, "%s - Sphere %s", name.c_str(), SPHERE_VERSION);
    }

    SetWindowTitle(caption);
  }

  CreateSystemObjects();

  SetFPSFont(&m_SystemFont);

  // load the game script
  std::string script = game_information.ReadString("", "script", "");

  if (script == "") {

    ShowError("No game script set.  Choose one from the editor.");

  } else {

    std::string script_path = "scripts/" + script;
    if (!m_Script->EvaluateFile(script_path.c_str(), m_FileSystem)) {

      ShowError(m_Script->GetError());

    } else {

      // execute the game
      std::string code = "game(";

      // escape quote characters within the string
      int parameters_length = strlen(m_Parameters);
      for (int i = 0; i < parameters_length; i++) {
        if (m_Parameters[i] == '"') {
          code += "\\\"";
        } else {
          code += m_Parameters[i];
        }
      }

      code += ")";
      if (!m_Script->Evaluate(code.c_str())) {
        ShowError(m_Script->GetError());
      }

    }
  }

  SetFPSFont(NULL);

  DestroySystemObjects();

  // destroy map engine
  delete m_MapEngine;
  m_MapEngine = NULL;

  delete m_Script;
  m_Script = 0;

  return m_NextGame;
}

////////////////////////////////////////////////////////////////////////////////

void Delay(int ms)
{
  dword end = GetTime() + ms;
  while (GetTime() < end) {
  }
}

void
CGameEngine::ShowError(const char* message)
{
  RGBA white = CreateRGBA(255, 255, 255, 255);

  SetClippingRectangle(0, 0, GetScreenWidth(), GetScreenHeight());

  bool done = false;
  bool keys_cleared = false;
  dword time = GetTime() + 1000;

  while (!done) {
    ClearScreen();
    m_SystemFont.DrawTextBox(0, 16, GetScreenWidth(), GetScreenHeight(), 0, message, white);

    //if (time < GetTime()) {
    //  int y = 16 + m_SystemFont.GetStringHeight(message, GetScreenWidth());
    //  m_SystemFont.DrawString(0, y, "Press Any Key to continue...", white);
    //}

    FlipScreen();

    if (time < GetTime()) {
      if (!keys_cleared) {
        ClearKeyQueue();
        keys_cleared = true;
      }

      if (AreKeysLeft()) {
        done = true;
      }

    }
  }
}

////////////////////////////////////////////////////////////////////////////////

inline static IMAGE CreateImage32(const CImage32& i) {
  return CreateImage(i.GetWidth(), i.GetHeight(), i.GetPixels());
}

void
CGameEngine::CreateSystemObjects()
{
  if (!m_SystemFont.CreateFromFont(m_SystemObjects.font)) {
    QuitMessage("Could not create system font");
  }

  if (!m_SystemWindowStyle.CreateFromWindowStyle(m_SystemObjects.window_style)) {
    QuitMessage("Could not create system window style");
  }

  m_SystemArrow = CreateImage32(m_SystemObjects.arrow);
  if (m_SystemArrow == NULL) {
    QuitMessage("Could not create system arrow");
  }

  m_SystemUpArrow = CreateImage32(m_SystemObjects.up_arrow);
  if (m_SystemUpArrow == NULL) {
    QuitMessage("Could not create system up arrow");
  }

  m_SystemDownArrow = CreateImage32(m_SystemObjects.down_arrow);
  if (m_SystemDownArrow == NULL) {
    QuitMessage("Could not create system down arrow");
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::DestroySystemObjects()
{
  m_SystemFont.Destroy();
  m_SystemWindowStyle.Destroy();
  if (m_SystemArrow)     DestroyImage(m_SystemArrow);     m_SystemArrow = NULL;
  if (m_SystemUpArrow)   DestroyImage(m_SystemUpArrow);   m_SystemUpArrow = NULL;
  if (m_SystemDownArrow) DestroyImage(m_SystemDownArrow); m_SystemDownArrow = NULL;
}

////////////////////////////////////////////////////////////////////////////////

// IEngine Methods

///////////////////////////////////////////////////////////////////////////////

bool
CGameEngine::IsScriptEvaluated(const char* filename)
{
  std::string name(filename);
  for (unsigned i = 0; i < m_EvaluatedScripts.size(); i++) {
    if (m_EvaluatedScripts[i] == name) {
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
CGameEngine::IsSystemScriptEvaluated(const char* filename)
{
  std::string name(filename);
  for (unsigned i = 0; i < m_EvaluatedSystemScripts.size(); i++) {
    if (m_EvaluatedSystemScripts[i] == name) {
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

void
CGameEngine::AddEvaluatedScript(const char* filename)
{
  if (!IsScriptEvaluated(filename)) {
    m_EvaluatedScripts.push_back(std::string(filename));
  }
}

///////////////////////////////////////////////////////////////////////////////
  
void
CGameEngine::AddEvaluatedSystemScript(const char* filename)
{
  if (!IsSystemScriptEvaluated(filename)) {
    m_EvaluatedSystemScripts.push_back(std::string(filename));
  }
}

///////////////////////////////////////////////////////////////////////////////

bool
CGameEngine::GetScriptText(const char* filename, std::string& text)
{
  // calculate path
  std::string path = "scripts/";
  path += filename;

  // open file
  std::auto_ptr<IFile> file(m_FileSystem.Open(path.c_str(), IFileSystem::read));
  if (!file.get()) {
    return false;
  }

  // read script
  int size = file->Size();
  char* script = new char[size + 1];
  file->Read(script, size);
  script[size] = 0;

  text = script;
  delete[] script;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CGameEngine::GetSystemScript(const char* filename, std::string& text)
{
  // switch to system script directory
  if (!EnterDirectory(m_SystemScriptDirectory.c_str())) {
    return false;
  }

  // open file
  std::auto_ptr<IFile> file(g_DefaultFileSystem.Open(filename, IFileSystem::read));
  if (!file.get()) {
    LeaveDirectory();
    return false;
  }

  // read script
  int size = file->Size();
  char* script = new char[size + 1];
  if (script == NULL)
    return false;

  file->Read(script, size);
  script[size] = 0;

  text = script;
  delete[] script;

  // go home
  LeaveDirectory();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::GetGameList(std::vector<Game>& games)
{
  games = m_GameList;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::ExecuteGame(const char* directory)
{
  m_NextGame = GetSphereDirectory() + "/games/" + directory;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::RestartGame()
{  
  m_RestartGame = true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CGameEngine::Restarted()
{
  return m_RestartGame;
}

////////////////////////////////////////////////////////////////////////////////

IEngine::script
CGameEngine::CompileScript(const char* script, std::string& error)
{
  // compile the script
  CScriptCode* so = m_Script->Compile(script);
  if (so == NULL) {
    error = m_Script->GetError();
    return NULL;
  }

  return so;
}

////////////////////////////////////////////////////////////////////////////////

#include <assert.h>

bool
CGameEngine::IsScriptBeingUsed(script s)
{
  assert(s);
  CScriptCode* so = (CScriptCode*)s;
  return so->IsBeingUsed();
}

////////////////////////////////////////////////////////////////////////////////

bool
CGameEngine::ExecuteScript(script s, bool& should_exit, std::string& error)
{
  CScriptCode* so = (CScriptCode*)s;

  // evaluating the script might invalidate the script object
  so->AddRef();

  if (so->Execute(should_exit) == false) {
    error = m_Script->GetError();
    so->Release();
    return false;
  }

  so->Release();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::DestroyScript(script s)
{
  CScriptCode* so = (CScriptCode*)s;
  so->Release();
}

////////////////////////////////////////////////////////////////////////////////

CMapEngine*
CGameEngine::GetMapEngine()
{
  return m_MapEngine;
}

////////////////////////////////////////////////////////////////////////////////

CLog*
CGameEngine::OpenLog(const char* filename)
{
  // make sure logs directory exists
  MakeDirectory("logs");

  std::string path = "logs/";
  path += filename;

  // open the log
  CLog* log = new CLog;
  if (log) {
    if (log->Open(path.c_str()) == false) {
      delete log;
      return NULL;
    }
  }

  return log;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::CloseLog(CLog* log)
{
  log->Close();
  delete log;
}

////////////////////////////////////////////////////////////////////////////////

SSPRITESET*
CGameEngine::LoadSpriteset(const char* filename)
{
  std::string path = "spritesets/";
  path += filename;

#ifndef USE_SPRITESET_SERVER
  // load spriteset
  SSPRITESET* ss = new SSPRITESET;
  if (!ss)
    return NULL;

  if (!ss->Load(path.c_str(), m_FileSystem, std::string(filename))) {
    ss->Release();
    return NULL;
  }

  return ss;
#else
  return m_SpritesetServer.Load(path.c_str(), m_FileSystem);
#endif
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::DestroySpriteset(SSPRITESET* spriteset)
{
#ifndef USE_SPRITESET_SERVER
  spriteset->Release();
#else
  m_SpritesetServer.Free(spriteset);
#endif
}

////////////////////////////////////////////////////////////////////////////////

SFONT*
CGameEngine::GetSystemFont()
{
  return &m_SystemFont;
}

////////////////////////////////////////////////////////////////////////////////

SFONT*
CGameEngine::LoadFont(const char* filename)
{
  std::string path = "fonts/";
  path += filename;

  // load font
  SFONT* font = new SFONT;
  if (!font)
    return NULL;

  if (!font->Load(path.c_str(), m_FileSystem)) {
    delete font;
    return NULL;
  }

  return font;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::DestroyFont(SFONT* font)
{
  delete font;
}

////////////////////////////////////////////////////////////////////////////////

SWINDOWSTYLE*
CGameEngine::GetSystemWindowStyle()
{
  return &m_SystemWindowStyle;
}

////////////////////////////////////////////////////////////////////////////////

SWINDOWSTYLE*
CGameEngine::LoadWindowStyle(const char* filename)
{
  std::string path = "windowstyles/";
  path += filename;

  // load window style
  SWINDOWSTYLE* ws = new SWINDOWSTYLE;
  if (!ws)
    return NULL;

  if (!ws->Load(path.c_str(), m_FileSystem)) {
    delete ws;
    return NULL;
  }

  return ws;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::DestroyWindowStyle(SWINDOWSTYLE* ws)
{
  delete ws;
  ws = NULL;
}

////////////////////////////////////////////////////////////////////////////////

class AudiereFile : public audiere::RefImplementation<audiere::File> {
public:
  AudiereFile(IFile* file) {
    m_file = file;
  }

  ~AudiereFile() {
    delete m_file;
  }

  int ADR_CALL read(void* buffer, int size) {
    return m_file->Read(buffer, size);
  }

  bool ADR_CALL seek(int position, SeekMode mode) {
    switch (mode) {
      case BEGIN:
        m_file->Seek(position);
        return true;

      case CURRENT:
        m_file->Seek(position + m_file->Tell());
        return true;

      case END:
        m_file->Seek(position + m_file->Size());
        return true;

      default:
        return false;
    }
  }

  int ADR_CALL tell() {
    return m_file->Tell();
  }

private:
  IFile* m_file;
};

////////////////////////////////////////////////////////////////////////////////

audiere::OutputStream*
CGameEngine::LoadSound(const char* filename, bool streaming)
{
  std::string path = "sounds/";
  path += filename;

  IFile* file = m_FileSystem.Open(path.c_str(), IFileSystem::read);
  if (!file) {
    return 0;
  }

  audiere::FilePtr adrfile(new AudiereFile(file));
  return SA_OpenSound(adrfile.get(), streaming);
}

////////////////////////////////////////////////////////////////////////////////

IMAGE
CGameEngine::GetSystemArrow()
{
  return m_SystemArrow;
}

////////////////////////////////////////////////////////////////////////////////

IMAGE
CGameEngine::GetSystemUpArrow()
{
  return m_SystemUpArrow;
}

////////////////////////////////////////////////////////////////////////////////

IMAGE
CGameEngine::GetSystemDownArrow()
{
  return m_SystemDownArrow;
}

////////////////////////////////////////////////////////////////////////////////

IMAGE
CGameEngine::LoadImage(const char* filename)
{
  std::string path = "images/";
  path += filename;

  // load the image
  CImage32 image;
  if (!image.Load(path.c_str(), m_FileSystem)) {
    return NULL;
  }

  return CreateImage(image.GetWidth(), image.GetHeight(), image.GetPixels());
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::DestroyImage(IMAGE image)
{
  ::DestroyImage(image);
}

////////////////////////////////////////////////////////////////////////////////

CImage32*
CGameEngine::LoadSurface(const char* filename)
{
  std::string path = "images/";
  path += filename;

  // load image
  CImage32* image = new CImage32;
  if (!image->Load(path.c_str(), m_FileSystem)) {
    delete image;
    return NULL;
  }

  return image;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::DestroySurface(CImage32* surface)
{
  delete surface;
  surface = NULL;
}

////////////////////////////////////////////////////////////////////////////////

IAnimation*
CGameEngine::LoadAnimation(const char* filename)
{
  std::string path = "animations/";
  path += filename;

  return ::LoadAnimation(path.c_str(), m_FileSystem);
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::GetFileList(const char* directory, std::vector<std::string>& vs)
{
  // enter the save directory
  //MakeDirectory(directory);
  if (!EnterDirectory(directory)) {
    return;
  }

  // now list directory contents
  FILELIST fl = BeginFileList("*");
  if (fl != NULL) {
    while (!FileListDone(fl)) {
      char file[FILENAME_MAX];
      GetNextFile(fl, file);
      vs.push_back(file);
    }

    EndFileList(fl);
  }

  LeaveDirectory();

  // if we're listing the save game directory files...
  if (strcmp(directory, "save") == 0) {
    // add currently open files too
    std::map<CConfigFile*, SFileInfo>::iterator i;
    for (i = m_OpenFiles.begin(); i != m_OpenFiles.end(); i++) {
      std::string filename = i->second.filename;

      // only insert the filename if it's not in the list
      std::vector<std::string>::iterator i = std::find(vs.begin(), vs.end(), filename);
      if (i == vs.end()) {
        vs.push_back(filename);
      } else {
        *i = filename;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

CConfigFile*
CGameEngine::OpenFile(const char* filename)
{
  // if file is already open, increase refcount and use it
  std::map<CConfigFile*, SFileInfo>::iterator i;
  for (i = m_OpenFiles.begin(); i != m_OpenFiles.end(); i++) {
    if (i->second.filename == filename) {
      i->second.refcount++;
      return i->first;
    }
  }

  // make sure save directory exists
  MakeDirectory("save");

  std::string path = "save/";
  path += filename;

  CConfigFile* file = new CConfigFile;
  if (!file)
    return NULL;

  file->Load(path.c_str(), m_FileSystem);

  // add the file to the opened files map
  SFileInfo fi;
  fi.filename = filename;
  fi.refcount = 1;
  m_OpenFiles[file] = fi;
  return file;
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::FlushFile(CConfigFile* file)
{
  // find where the file is in the map
  std::map<CConfigFile*, SFileInfo>::iterator i;
  for (i = m_OpenFiles.begin(); i != m_OpenFiles.end(); i++) {
    if (i->first == file) {
      // save it
      std::string path = "save/";
      path += i->second.filename;

      i->first->Save(path.c_str(), m_FileSystem);
      break;
    }
  }  
}

////////////////////////////////////////////////////////////////////////////////

void
CGameEngine::CloseFile(CConfigFile* file)
{
  // find where the file is in the map
  std::map<CConfigFile*, SFileInfo>::iterator i;
  for (i = m_OpenFiles.begin(); i != m_OpenFiles.end(); i++) {
    if (i->first == file) {
      if (--(i->second.refcount) == 0) {

        // save and destroy it
        std::string path = "save/";
        path += i->second.filename;

        i->first->Save(path.c_str(), m_FileSystem);
        delete i->first;
        m_OpenFiles.erase(i);
        break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IFile*
CGameEngine::OpenRawFile(const char* filename, bool writeable)
{
  int mode = (writeable ? IFileSystem::write : IFileSystem::read);
  if (writeable)
    MakeDirectory("other");

  std::string path = "other/";
  path += filename;

  return m_FileSystem.Open(path.c_str(), mode);
}

////////////////////////////////////////////////////////////////////////////////
