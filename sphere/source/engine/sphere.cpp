// identifier too long
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sphere.hpp"
#include "benchmark.hpp"
#include "engine.hpp"
#include "render.hpp"
#include "inputx.hpp"

#include "audio.hpp"
#include "filesystem.hpp"
#include "input.hpp"
#include "system.hpp"
#include "time.hpp"

#include "../common/DefaultFileSystem.hpp"
#include "../common/PackageFileSystem.hpp"
#include "../common/configfile.hpp"


static void RunPackage(IFileSystem& fs);
static void RunGame(const char* game, const char* parameters);
static void GetGameList(std::vector<Game>& games);
static void LoadSystemObjects();
static void DestroySystemObjects();


static SSystemObjects    s_SystemObjects;
static std::vector<Game> s_GameList;
static std::string       s_ScriptDirectory;


////////////////////////////////////////////////////////////////////////////////

void RunSphere(int argc, const char** argv)
{
  // populate the game list
  GetGameList(s_GameList);

  // load system objects (and store script directory)
  LoadSystemObjects();

  bool show_menu = true;

  // check for manual game selection
  for (int i = 1; i < argc; i++) {

    if (strcmp(argv[i], "-benchmark") == 0) {

      // run video benchmarks
      BeginBenchmarks(s_SystemObjects);
      show_menu = false;

    } else if (strcmp(argv[i], "-game") == 0 &&
               i < argc - 1) {  // if last parameter is a command, it doesn't mean anything

      // look for a parameters string
      const char* parameters = "";
      for (int j = 0; j < argc - 1; j++) {
        if (strcmp(argv[j], "-parameters") == 0) {
          parameters = argv[j + 1];
          break;
        }
      }

      // run the game
      if (EnterDirectory("games")) {
        RunGame(argv[i + 1], parameters);
        LeaveDirectory();
      } else {
        QuitMessage("could not enter 'games' directory");
      }
      show_menu = false;

    } else if (strcmp(argv[i], "-package") == 0 &&
               i < argc - 1) {  // if last parameter is a command, it doesn't mean anything

      const char* package_name = argv[i + 1];
      if (strrchr(package_name, '/')) {
        package_name = strrchr(package_name, '/') + 1;
      } else if (strrchr(package_name, '\\')) {
        package_name = strrchr(package_name, '\\') + 1;
      }

      // make sure "packages" directory exists
      MakeDirectory("packages");
      EnterDirectory("packages");

      // now make a directory based on the name of the package
      MakeDirectory(package_name);
      EnterDirectory(package_name);

      // run the package
      CPackageFileSystem fs(argv[i + 1]);
      if (fs.GetNumFiles() == 0) {
        std::ostringstream os;
        os << "Could not open package '" << argv[i + 1] << "'";
        QuitMessage(os.str().c_str());
      }

      RunPackage(fs);
      show_menu = false;

      LeaveDirectory();
      LeaveDirectory();

    }
  }

  // start the game specified on the command line
  if (show_menu) {

    RunGame("startup", "");

  }

  DestroySystemObjects();
}

////////////////////////////////////////////////////////////////////////////////

void RunPackage(IFileSystem& fs)
{
  CGameEngine(fs, s_SystemObjects, s_GameList, s_ScriptDirectory.c_str(), "").Run();
  ClearKeyQueue();
}

////////////////////////////////////////////////////////////////////////////////

std::string DoRunGame(const char* game, const char* parameters) {
  printf("Game: %s\n", game);
  printf("Parameters: %s\n", parameters);

  std::string result;
  if (EnterDirectory(game)) {
    result = CGameEngine(
      g_DefaultFileSystem,
      s_SystemObjects,
      s_GameList,
      s_ScriptDirectory.c_str(),
      parameters
    ).Run();
    ClearKeyQueue();
    LeaveDirectory();
  } else {
    QuitMessage("Could not enter game directory");
  }

  return result;
}

void RunGame(const char* game, const char* parameters) {
  // first = game directory, second = parameters
  typedef std::pair<std::string, std::string> Game;
  std::stack<Game> games;
  games.push(Game(game, parameters));

  while (!games.empty()) {
    Game g = games.top();
    games.pop();

    std::string result = DoRunGame(g.first.c_str(), g.second.c_str());
    if (!result.empty()) {
      // add the original game back to the stack
      games.push(g);
      games.push(Game("games/" + result, ""));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void GetGameList(std::vector<Game>& games)
{
  if (!EnterDirectory("games")) {
    return;
  }

  // add game directories
  DIRECTORYLIST dl = BeginDirectoryList("*");

  while (!DirectoryListDone(dl)) {
    char directory[520];
    GetNextDirectory(dl, directory);

    puts(directory);

    if (EnterDirectory(directory)) {

      // read the game name
      CConfigFile file;
      file.Load("game.sgm");
      std::string gamename = file.ReadString("", "name", "");

      // if the game name is empty, the game doesn't exist
      if (gamename.length() != 0) {
        Game g;
        g.name = gamename;
        g.directory = directory;
        games.push_back(g);
      }

      LeaveDirectory();
    }
  }

  EndDirectoryList(dl);

  // add game packages

  // alphabetize the menu
  std::sort(games.begin(), games.end());

  LeaveDirectory();
}

////////////////////////////////////////////////////////////////////////////////

void LoadSystemObjects()
{
  if (!EnterDirectory("system")) {
    QuitMessage("System directory not found");
  }

  // store system scripts directory
  if (!EnterDirectory("scripts")) {
    QuitMessage("System scripts directory not found");
  }
  GetDirectory(s_ScriptDirectory);
  LeaveDirectory();

  CConfigFile file("system.ini");
  std::string font         = file.ReadString("", "Font",        "unknown");
  std::string window_style = file.ReadString("", "WindowStyle", "unknown");
  std::string arrow        = file.ReadString("", "Arrow",       "unknown");
  std::string up_arrow     = file.ReadString("", "UpArrow",     "unknown");
  std::string down_arrow   = file.ReadString("", "DownArrow",   "unknown");

  // system font
  if (!s_SystemObjects.font.Load(font.c_str(), g_DefaultFileSystem)) {
    QuitMessage("Error: Could not load system font");
  }

  // system window style
  if (!s_SystemObjects.window_style.Load(window_style.c_str())) {
    QuitMessage("Error: Could not load system window style");
  }

  // system arrow
  if (!s_SystemObjects.arrow.Load(arrow.c_str())) {
    QuitMessage("Error: Could not load system arrow");
  }

  // system up arrow
  if (!s_SystemObjects.up_arrow.Load(up_arrow.c_str())) {
    QuitMessage("Error: Could not load system up arrow");
  }

  // system down arrow
  if (!s_SystemObjects.down_arrow.Load(down_arrow.c_str())) {
    QuitMessage("Error: Could not load system down arrow");
  }

  LeaveDirectory();
}

////////////////////////////////////////////////////////////////////////////////

void DestroySystemObjects()
{
}

////////////////////////////////////////////////////////////////////////////////
