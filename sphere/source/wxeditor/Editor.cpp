#include "Editor.hpp"
#include "MainWindow.hpp"
#include "Audio.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "Debug.hpp"
#include "../engine/win32/win32_sphere_config.hpp"


static EditorApplication g_Application;


std::string g_SphereDirectory;

static wxStatusBar* s_StatusBar;
static wMainWindow *s_MainWindow;

/*todo:
// command-line parser

class CEditorCommandLineInfo : public CCommandLineInfo
{
public:
  CEditorCommandLineInfo(CMainWindow* mainwindow)
  : m_MainWindow(mainwindow)
  {
  }

private:
  void ParseParam(LPCTSTR parameter, BOOL /*flag* /, BOOL /*last* /)
  {
    m_MainWindow->OpenGameFile(parameter);
  }

private:
  CMainWindow* m_MainWindow;
};
  
*/

////////////////////////////////////////////////////////////////////////////////

EditorApplication::EditorApplication()
{
  SetAppName(wxString("Sphere Editor"));
  InitializeLog();
}

////////////////////////////////////////////////////////////////////////////////

bool
EditorApplication::OnInit()
{
  if(!wxApp::OnInit()) {
    return FALSE;
  }
  // set the configuration directory
  //char config_directory[MAX_PATH];
  //GetModuleFileName(m_hInstance, config_directory, MAX_PATH);
  //if (strrchr(config_directory, '\\'))
  //  *strrchr(config_directory, '\\') = 0;
  //Configuration::ConfigurationFile = std::string(config_directory) + "\\editor.ini";

  wxString config_directory =  wxGetCwd();
  Configuration::ConfigurationFile = std::string(config_directory) + "editor.ini";//todo: verify this will produce the correct filename

  // get the startup directory
  // THE STUB WILL TAKE CARE OF STARTING THE EXECUTABLE IN THE RIGHT WORKING DIR
  //char directory[MAX_PATH];
  //GetCurrentDirectory(MAX_PATH, directory);
  //g_SphereDirectory = directory;
  //SetCurrentDirectory(directory);

  wxString directory = wxGetCwd();
  g_SphereDirectory = std::string(directory.c_str());
  wxSetWorkingDirectory(directory);

  // create the main window
  wMainWindow* main_window = new wMainWindow("Sphere", wxDefaultPosition, wxSize(640, 480), wxDEFAULT_FRAME_STYLE | wxMAXIMIZE);

  SetMainWindow(main_window);

  //main_window->Create();
  main_window->Show();
  SetMainWindow(main_window);


  SPHERECONFIG sphere_config;
  LoadSphereConfig(&sphere_config, "engine.ini");

/*todo:
  // parse the command line
  CEditorCommandLineInfo cli(main_window);
  ParseCommandLine(cli);
*/

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

int
EditorApplication::OnExit()
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void SetStatusBar(wxStatusBar* status_bar)
{
  s_StatusBar = status_bar;
}

////////////////////////////////////////////////////////////////////////////////

wxStatusBar* GetStatusBar()
{
  return s_StatusBar;
}

////////////////////////////////////////////////////////////////////////////////

void SetStatus(wxString &text, int i) {
  s_StatusBar->SetStatusText(text, i);
}

////////////////////////////////////////////////////////////////////////////////

void SetMainWindow(wMainWindow *mainwindow) {
  s_MainWindow = mainwindow;
}

////////////////////////////////////////////////////////////////////////////////

wMainWindow *GetMainWindow() {
  return s_MainWindow;
}

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_APP(EditorApplication);

