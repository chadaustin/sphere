#include "Editor.hpp"
#include "MainWindow.hpp"
#include "Audio.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "Debug.hpp"
#include "FolderDialog.hpp"
#include "../engine/win32/win32_sphere_config.hpp"


static CEditorApplication g_Application;


std::string g_SphereDirectory;

static CStatusBar* s_StatusBar;


// command-line parser

class CEditorCommandLineInfo : public CCommandLineInfo
{
public:
  CEditorCommandLineInfo(CMainWindow* mainwindow)
  : m_MainWindow(mainwindow)
  {
  }

private:
  void ParseParam(LPCTSTR parameter, BOOL /*flag*/, BOOL /*last*/)
  {
    m_MainWindow->OpenGameFile(parameter);
  }

private:
  CMainWindow* m_MainWindow;
};
  

////////////////////////////////////////////////////////////////////////////////

CEditorApplication::CEditorApplication()
: CWinApp("Sphere Editor")
{
  InitializeLog();
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CEditorApplication::InitInstance()
{
  // set the configuration directory
  char config_directory[MAX_PATH];
  GetModuleFileName(m_hInstance, config_directory, MAX_PATH);
  if (strrchr(config_directory, '\\'))
    *strrchr(config_directory, '\\') = 0;
  Configuration::ConfigurationFile = std::string(config_directory) + "\\editor.ini";

  // get the startup directory
  // THE STUB WILL TAKE CARE OF STARTING THE EXECUTABLE IN THE RIGHT WORKING DIR
  char directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, directory);

  g_SphereDirectory = directory;
  SetCurrentDirectory(directory);

  // create the main window
  CMainWindow* main_window = new CMainWindow();
  main_window->Create();
  m_pMainWnd = main_window;

  SPHERECONFIG sphere_config;
  LoadSphereConfig(&sphere_config, "engine.ini");

  // parse the command line
  CEditorCommandLineInfo cli(main_window);
  ParseCommandLine(cli);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

int
CEditorApplication::ExitInstance()
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void SetStatusBar(CStatusBar* status_bar)
{
  s_StatusBar = status_bar;
}

////////////////////////////////////////////////////////////////////////////////

CStatusBar* GetStatusBar()
{
  return s_StatusBar;
}

////////////////////////////////////////////////////////////////////////////////
