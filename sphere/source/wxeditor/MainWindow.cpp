#ifdef _MSC_VER
// disable 'identifier too long' warning
#pragma warning(disable : 4786)
#endif

// standard
#include <set>

// core
#include "Editor.hpp"
#include "WindowCommands.hpp"
#include "FileSystem.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "Package.hpp"
//#include "resource.h"
#include "IDs.hpp"

// document windows
#include "MainWindow.hpp"
#include "ProjectWindow.hpp"
#include "MapWindow.hpp"
#include "SpritesetWindow.hpp"
#include "SoundWindow.hpp"
#include "ScriptWindow.hpp"
#include "ImageWindow.hpp"
#include "AnimationWindow.hpp"
#include "WindowStyleWindow.hpp"
#include "FontWindow.hpp"

// dialogs
#include "NewProjectDialog.hpp"
//#include "OptionsDialog.hpp"
#include "GameSettingsDialog.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"

// common
#include "../common/sphere_version.h"
#include "../common/Map.hpp"
#include "../common/strcmp_ci.hpp"

// system
#include "../common/system.hpp"

// libraries
// libraries are down here because of symbol conflicts with other headers
#include <jsapi.h>
#include <zlib.h>
#include <libmng.h>

#include "icons/sph-game.xpm"

// base for palette menu items
const int PALETTE_COMMAND = 17133;
const int NUM_PALETTES    = 100;


const char szBarState[] = "SDE_BarState";

BEGIN_EVENT_TABLE(wMainWindow, wxMDIParentFrame)
  EVT_CLOSE(wMainWindow::OnClose)
  // generic file open
  EVT_MENU(wID_FILE_OPEN, wMainWindow::OnFileOpen)

  // project
  EVT_MENU(wID_FILE_NEW_PROJECT,     wMainWindow::OnFileNewProject)
  EVT_MENU(wID_FILE_OPEN_PROJECT,    wMainWindow::OnFileOpenProject)
  EVT_MENU(wID_FILE_CLOSEPROJECT,    wMainWindow::OnFileCloseProject)
  EVT_MENU(wID_FILE_OPENLASTPROJECT, wMainWindow::OnFileOpenLastProject)

  // file | new
  EVT_MENU(wID_FILE_NEW_MAP,         wMainWindow::OnFileNewMap)
  EVT_MENU(wID_FILE_NEW_SPRITESET,   wMainWindow::OnFileNewSpriteset)
  EVT_MENU(wID_FILE_NEW_SCRIPT,      wMainWindow::OnFileNewScript)
  EVT_MENU(wID_FILE_NEW_FONT,        wMainWindow::OnFileNewFont)
  EVT_MENU(wID_FILE_NEW_WINDOWSTYLE, wMainWindow::OnFileNewWindowStyle)
  EVT_MENU(wID_FILE_NEW_IMAGE,       wMainWindow::OnFileNewImage)

  // file | open
  EVT_MENU(wID_FILE_OPEN_MAP,         wMainWindow::OnFileOpenMap)
  EVT_MENU(wID_FILE_OPEN_SPRITESET,   wMainWindow::OnFileOpenSpriteset)
  EVT_MENU(wID_FILE_OPEN_SCRIPT,      wMainWindow::OnFileOpenScript)
  EVT_MENU(wID_FILE_OPEN_SOUND,       wMainWindow::OnFileOpenSound)
  EVT_MENU(wID_FILE_OPEN_FONT,        wMainWindow::OnFileOpenFont)
  EVT_MENU(wID_FILE_OPEN_WINDOWSTYLE, wMainWindow::OnFileOpenWindowStyle)
  EVT_MENU(wID_FILE_OPEN_IMAGE,       wMainWindow::OnFileOpenImage)
  EVT_MENU(wID_FILE_OPEN_ANIMATION,   wMainWindow::OnFileOpenAnimation)

  EVT_MENU(wID_FILE_CLOSE,            wMainWindow::OnFileClose)

  // file | import
  EVT_MENU(wID_FILE_IMPORT_IMAGETOMAPTILESET, wMainWindow::OnFileImportImageToMap)
  EVT_MENU(wID_FILE_IMPORT_BITMAPTORWS,       wMainWindow::OnFileImportBitmapToRWS)
  EVT_MENU(wID_FILE_IMPORT_BITMAPTORSS,       wMainWindow::OnFileImportBitmapToRSS)
  EVT_MENU(wID_FILE_IMPORT_BITMAPTORTS,       wMainWindow::OnFileImportBitmapToRTS)
  EVT_MENU(wID_FILE_IMPORT_VERGEFONTTEMPLATE, wMainWindow::OnFileImportVergeFontTemplate)
  EVT_MENU(wID_FILE_IMPORT_VERGEMAP,          wMainWindow::OnFileImportVergeMap)
  EVT_MENU(wID_FILE_IMPORT_VERGESPRITESET,    wMainWindow::OnFileImportVergeSpriteset)
  EVT_MENU(wID_FILE_IMPORT_MERGE_RGBA,        wMainWindow::OnFileImportMergeRGBA)
  EVT_MENU(wID_FILE_IMPORT_WINDOWSFONT,       wMainWindow::OnFileImportWindowsFont)

  EVT_MENU(wID_FILE_SAVEALL, wMainWindow::OnFileSaveAll)

  EVT_MENU(wID_FILE_OPTIONS, wMainWindow::OnFileOptions)
  EVT_MENU(wID_FILE_EXIT,    wMainWindow::OnClose)

  // insert
  EVT_MENU(wID_PROJECT_INSERT_MAP,         wMainWindow::OnProjectInsertMap)
  EVT_MENU(wID_PROJECT_INSERT_SPRITESET,   wMainWindow::OnProjectInsertSpriteset)
  EVT_MENU(wID_PROJECT_INSERT_SCRIPT,      wMainWindow::OnProjectInsertScript)
  EVT_MENU(wID_PROJECT_INSERT_SOUND,       wMainWindow::OnProjectInsertSound)
  EVT_MENU(wID_PROJECT_INSERT_FONT,        wMainWindow::OnProjectInsertFont)
  EVT_MENU(wID_PROJECT_INSERT_WINDOWSTYLE, wMainWindow::OnProjectInsertWindowStyle)
  EVT_MENU(wID_PROJECT_INSERT_IMAGE,       wMainWindow::OnProjectInsertImage)
  EVT_MENU(wID_PROJECT_INSERT_ANIMATION,   wMainWindow::OnProjectInsertAnimation)

  EVT_MENU(wID_PROJECT_REFRESH,         wMainWindow::OnProjectRefresh)
  EVT_MENU(wID_PROJECT_RUNSPHERE,       wMainWindow::OnProjectRunSphere)
  EVT_MENU(wID_PROJECT_CONFIGURESPHERE, wMainWindow::OnProjectConfigureSphere)
  EVT_MENU(wID_PROJECT_PACKAGE_GAME,    wMainWindow::OnProjectPackageGame)

  EVT_MENU(wID_WINDOW_CLOSEALL, wMainWindow::OnWindowCloseAll)

  EVT_MENU(wID_HELP_CONTENTS, wMainWindow::OnHelpContents)
  EVT_MENU(wID_HELP_ABOUT,    wMainWindow::OnHelpAbout)

END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////

wMainWindow::wMainWindow(
                       const wxString& title, const wxPoint& pos,
                       const wxSize& size, long type) 
: wxMDIParentFrame(NULL, -1, title, pos, size, type, "SDE_MainWindow")
, m_ProjectOpen(false)
, m_ProjectWindow(NULL)
, m_ChildMenuResource(-1)
, m_PreviousChildMenuResource(-1)
, m_MenuReceiver(NULL)
, m_InsideEventProcess(0)
{
  wxIcon icon(sph_game_xpm);
  SetIcon(icon);

  m_MenuBar = new wxMenuBar();

  wxMenu *menu;
  wxMenu *submenu;

  menu = new wxMenu();

    submenu = new wxMenu();//"New");
      submenu->Append(wID_FILE_NEW_PROJECT,     "New &Project");
      submenu->AppendSeparator();
      submenu->Append(wID_FILE_NEW_MAP,         "&Map");
      submenu->Append(wID_FILE_NEW_SPRITESET,   "&Spriteset");
      submenu->Append(wID_FILE_NEW_SCRIPT,      "S&cript");
      submenu->Append(wID_FILE_NEW_FONT,        "&Font");
      submenu->Append(wID_FILE_NEW_WINDOWSTYLE, "&WindowStyle");
      submenu->Append(wID_FILE_NEW_IMAGE,       "&Image");
    menu->Append(wID_FILE_NEW_, "&New", submenu);

    submenu = new wxMenu();//"Open");
      submenu->Append(wID_FILE_OPEN_PROJECT,     "Open &Project");
      submenu->AppendSeparator();
      submenu->Append(wID_FILE_OPEN_MAP,         "&Map");
      submenu->Append(wID_FILE_OPEN_SPRITESET,   "&Spriteset");
      submenu->Append(wID_FILE_OPEN_SCRIPT,      "S&cript");
      submenu->Append(wID_FILE_OPEN_SOUND,       "S&ound");
      submenu->Append(wID_FILE_OPEN_FONT,        "&Font");
      submenu->Append(wID_FILE_OPEN_WINDOWSTYLE, "&WindowStyle");
      submenu->Append(wID_FILE_OPEN_IMAGE,       "&Image");
      submenu->Append(wID_FILE_OPEN_ANIMATION,   "&Animation");
      submenu->AppendSeparator();
      submenu->Append(wID_FILE_OPEN, "&Open");
    menu->Append(wID_FILE_OPEN_, "&Open", submenu);

    menu->Append(wID_FILE_CLOSE, "&Close");

    submenu = new wxMenu();//"Import");
      submenu->Append(wID_FILE_IMPORT_IMAGETOMAPTILESET, "Image To &Map");
      submenu->Append(wID_FILE_IMPORT_BITMAPTORWS,       "Bitmap To R&WS (WindowStyle)");
      submenu->Append(wID_FILE_IMPORT_BITMAPTORSS,       "Bitmap To R&SS (Spriteset)");
      submenu->Append(wID_FILE_IMPORT_BITMAPTORTS,       "Bitmap To R&TS (Tileset)");
      submenu->Append(wID_FILE_IMPORT_VERGEFONTTEMPLATE, "Verge &Font Template");
      submenu->Append(wID_FILE_IMPORT_VERGEMAP,          "Verge M&ap");
      submenu->Append(wID_FILE_IMPORT_VERGESPRITESET,    "Verge S&priteset");
      submenu->Append(wID_FILE_IMPORT_MERGE_RGBA,        "Merge RGBA");
      submenu->Append(wID_FILE_IMPORT_WINDOWSFONT,       "&Windows Font");
    menu->Append(wID_FILE_IMPORT_, "&Import", submenu);

    menu->Append(wID_FILE_OPENLASTPROJECT, "Open &Previous Project");
    menu->Append(wID_FILE_CLOSEPROJECT,    "&Close Project");

    menu->AppendSeparator();

    menu->Append(wxID_SAVE, "&Save");
    menu->Append(wxID_SAVEAS, "Save &As");
    //menu->Append(wID_FILE_SAVE, "&Save");
    //menu->Append(wID_FILE_SAVEAS, "Save &As");
    menu->Append(wID_FILE_SAVEALL, "Save A&ll");

    menu->AppendSeparator();

    menu->Append(wID_FILE_OPTIONS, "Options");
    menu->Append(wID_FILE_EXIT,    "E&xit");

  m_MenuBar->Append(menu, "&File");

  SetMenuBar(m_MenuBar);


  // create the statusbar
  m_StatusBar.Create(this, -1);
  m_StatusBar.SetFieldsCount(1);

  SetStatusBar(&m_StatusBar);

  ::SetStatusBar(&m_StatusBar);

}

////////////////////////////////////////////////////////////////////////////////
#if 0

todo:

bool
wMainWindow::Create()
{
/*
  // create the window
  CMDIFrameWnd::Create(
    AfxRegisterWndClass(0, NULL, NULL, LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_SDE))),
    "Sphere Development Environment",
    WS_OVERLAPPEDWINDOW,
    rectDefault,
    NULL,
    MAKEINTRESOURCE(IDR_MAIN));
*/


/*
  LoadAccelTable(MAKEINTRESOURCE(IDR_ACCELERATOR));
*/

/*
  // create the toolbar
  m_ToolBar.CreateEx(
    this,
    TBSTYLE_FLAT,
    WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC | CBRS_TOP | CBRS_GRIPPER | CBRS_FLYBY | CBRS_TOOLTIPS);
  m_ToolBar.SetWindowText("Main");
  m_ToolBar.LoadToolBar(IDR_TOOLBAR);
  m_ToolBar.EnableDocking(CBRS_ALIGN_ANY);
*/

/*
  // status bar indicators
  static const UINT indicators[] =
  {
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
  };

  // create the statusbar
  m_StatusBar.Create(this);
  m_StatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
  m_StatusBar.SetBarStyle(m_StatusBar.GetBarStyle() | CBRS_FLYBY | CBRS_TOOLTIPS);

  SetStatusBar(&m_StatusBar);
*/
  CreateStatusBar(3);


  // enable docking
  EnableDocking(CBRS_ALIGN_ANY);

  DockControlBar(&m_ToolBar, AFX_IDW_DOCKBAR_TOP);
  
  // load the command bar state
  LoadBarState(szBarState);

  // enable drag and drop
  DragAcceptFiles(true);

  // show the window in the initial position
  WINDOWPLACEMENT wp = Configuration::Get(KEY_STARTUP);
  if (wp.length != 0) {
    SetWindowPlacement(&wp);
  } else {
    ShowWindow(SW_SHOW);
  }

  UpdateWindow();
  UpdateMenu();
  UpdatePaletteMenu();

  return TRUE;
}
#endif

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::CreateProject(const char* projectname, const char* gametitle)
{
  CloseProject();

  char games_directory[MAX_PATH];
  GetGamesDirectory(games_directory);

  // this may fail, but we don't care
  CreateDirectory(games_directory, NULL);
  
  if (!m_Project.Create(games_directory, projectname))
  {
    ::wxMessageBox("Error: Could not create project");
    return;
  }

  m_Project.SetGameTitle(gametitle);
  m_ProjectOpen = true;
  m_ProjectWindow = new wProjectWindow(this, &m_Project);
  //m_ProjectWindow->Create();

  UpdateMenu();
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OpenProject(const char* filename)
{
  CloseProject();
  
  if (m_Project.Open(filename) == false)
  {
    ::wxMessageBox("Could not open project");
    return;
  }

  Configuration::Set(KEY_LAST_PROJECT(), filename);
  
  m_ProjectOpen = true;
  m_ProjectWindow = new wProjectWindow(this, &m_Project);
  //m_ProjectWindow->Create();

  UpdateMenu();
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::CloseProject()
{
  if (m_ProjectOpen)
  {
    m_ProjectOpen = false;

    if (m_ProjectWindow)
    {
      m_ProjectWindow->Destroy();
      m_ProjectWindow = NULL;
    }

    UpdateMenu();
  }
}

////////////////////////////////////////////////////////////////////////////////

void 
wMainWindow::CloseNotice(wDocumentWindow *doc)
{
  // remove window from list
  for (unsigned i = 0; i < m_DocumentWindows.size(); i++) {
    if (m_DocumentWindows[i] == doc)
    {
      m_DocumentWindows.erase(m_DocumentWindows.begin() + i);
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OpenGameSettings()
{
  wGameSettingsDialog(this, &m_Project).ShowModal();
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OpenGameFile(const char* filename)
{
  // test if it's a project file
  static const char* game_sgm = "game.sgm";
  int filename_length = strlen(filename);
  int game_sgm_length = strlen(game_sgm);
  if (filename_length > game_sgm_length && strcmp(filename + filename_length - game_sgm_length, game_sgm) == 0) {
    OpenProject(filename);
  }

  for (int i = 0; i < NUM_GROUP_TYPES; i++) {
    std::vector<std::string> extensions;
    FTL.GetFileTypeExtensions(i, extensions);
    
    for (unsigned k = 0; k < extensions.size(); k++) {
      std::string ext = "." + extensions[k];
      if (strcmp_ci(filename + strlen(filename) - ext.length(), ext.c_str()) == 0) {
        OpenDocumentWindow(i, filename);
        return;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OpenDocumentWindow(int grouptype, const char* filename)
{
  // if a document window has the same filename, just give it focus
  for (unsigned i = 0; i < m_DocumentWindows.size(); i++) {
    if (strcmp(m_DocumentWindows[i]->GetFilename(), filename) == 0) {
      m_DocumentWindows[i]->SetFocus();
      return;
    }
  }

  // actually open the window now
  wDocumentWindow* window = NULL;
  switch (grouptype)
  {
    case GT_MAPS:         window = new wMapWindow(filename);         break;
    case GT_SPRITESETS:   window = new wSpritesetWindow(filename);   break;
    case GT_SCRIPTS:      window = new wScriptWindow(filename);      break;
    case GT_SOUNDS:       window = new wSoundWindow(filename);       break;
    case GT_FONTS:        window = new wFontWindow(filename);        break;
    case GT_WINDOWSTYLES: window = new wWindowStyleWindow(filename); break;
    case GT_IMAGES:       window = new wImageWindow(filename);       break;
    case GT_ANIMATIONS:   window = new wAnimationWindow(filename);   break;
  }

  m_DocumentWindows.push_back(window);
  window->SetFocus();
}

////////////////////////////////////////////////////////////////////////////////

bool
wMainWindow::AddToDirectory(const char* pathname, const char* sub_directory)
{
  // if file isn't already in the subdirectory
  if (!CheckDirectory(pathname, sub_directory))
  {
    // ask the user if we can copy it
    char message[1024];
    sprintf(message, "The file must be copied into the game '%s' directory.  Is this okay?", sub_directory);
    if (::wxMessageBox(message, "Confirm copy", wxYES_NO) == wxNO)
      return false;

    char szDestination[MAX_PATH];
    strcpy(szDestination, m_Project.GetDirectory());
    strcat(szDestination, "\\");
    strcat(szDestination, sub_directory);

    // create the directory
    CreateDirectory(szDestination, NULL);

    // append the filename
    strcat(szDestination, "\\");
    strcat(szDestination, strrchr(pathname, '\\') + 1);

    // copy it
    if (CopyFile(pathname, szDestination, TRUE) == FALSE)
    {
      if (::wxMessageBox("File appears to already exist, overwrite?", "Overwrite?", wxYES_NO) == wxNO)
        return false;

      if (CopyFile(pathname, szDestination, FALSE) == FALSE)
      {
        ::wxMessageBox("Error: Could not copy file");
        return false;
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
wMainWindow::CheckDirectory(const char* filename, const char* sub_directory)
{
  // remove the file name from szFileName
  char szDirectory[MAX_PATH];
  strcpy(szDirectory, filename);
  if (*strrchr(szDirectory, '\\'))
    *strrchr(szDirectory, '\\') = 0;

  // add the sub directory to the project directory
  char szProjectDirectory[MAX_PATH];
  strcpy(szProjectDirectory, m_Project.GetDirectory());
  strcat(szProjectDirectory, "\\");
  strcat(szProjectDirectory, sub_directory);

  // compare the path with the project directory + szSubDirectory
#ifdef WIN32
  strlwr(szDirectory);
  strlwr(szProjectDirectory);
#endif
  return (strcmp(szDirectory, szProjectDirectory) == 0);
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::InsertProjectFile(wxFileDialog* file_dialog, int grouptype, const char* predefined_path)
{
  // determine filetype directory
  char path[MAX_PATH];
  sprintf(path, "%s\\%s", m_Project.GetDirectory(), m_Project.GetGroupDirectory(grouptype));
  if (!PathExists(path))
  {
    char szMessage[80];
    sprintf(szMessage, "'%s' subdirectory does not exist, create?", m_Project.GetGroupDirectory(grouptype));
    if (::wxMessageBox(szMessage, "Create directory?", wxYES_NO) == wxNO)
      return;

    // create the directory
    if (!CreateDirectory(path, NULL))
    {
      char szMessage[MAX_PATH + 80];
      sprintf(szMessage, "Error: Could not create directory '%s'", path);
      ::wxMessageBox(szMessage);
      return;
    }
  }

  wxString sPathName;

  if (file_dialog) {
    // start the file dialog in the correct directory
    SetCurrentDirectory(path);
    if (file_dialog->ShowModal() != wxID_OK)
      return;

    // we've got a full file path
    sPathName = file_dialog->GetPath();
  } else {
    sPathName = predefined_path;
  }

  char path_name[MAX_PATH];
  strcpy(path_name, sPathName);
  *strrchr(path_name, '\\') = 0;

  if (strcmp_ci(path, path_name) == 0)
  {
    // if the file is in the same directory, and the file already exists
    if (FileExists(sPathName))
    {
      ::wxMessageBox("File is already in project");
      return;
    }
    else
    {
      // create an empty file that will not be valid
      fclose(fopen(sPathName, "wb"));
    }
  }
  else
  {
    // if file exists, use AddToDirectory() in case we need to copy it
    AddToDirectory(sPathName, wProject::GetGroupDirectory(grouptype));
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::GetGamesDirectory(char games_directory[MAX_PATH])
{
  strcpy(games_directory, g_SphereDirectory.c_str());
#ifdef WIN32
  if (games_directory[strlen(games_directory) - 1] != '\\')
    strcat(games_directory, "\\");
#else
  if (games_directory[strlen(games_directory) - 1] != '/')
    strcat(games_directory, "/");
#endif
  strcat(games_directory, "games");
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::UpdateMenu()
{
  if(m_ProjectOpen) {
    if(m_MenuBar->FindMenu("&Project") == wxNOT_FOUND) {
      wxMenu *projectmenu = new wxMenu();
      projectmenu->Append(wID_PROJECT_INSERT_MAP,         "Insert Map");
      projectmenu->Append(wID_PROJECT_INSERT_SPRITESET,   "Insert Spriteset");
      projectmenu->Append(wID_PROJECT_INSERT_SCRIPT,      "Insert Script");
      projectmenu->Append(wID_PROJECT_INSERT_SOUND,       "Insert Sound");
      projectmenu->Append(wID_PROJECT_INSERT_FONT,        "Insert Font");
      projectmenu->Append(wID_PROJECT_INSERT_WINDOWSTYLE, "Insert Windowstyle");
      projectmenu->Append(wID_PROJECT_INSERT_IMAGE,       "Insert Image");
      projectmenu->Append(wID_PROJECT_INSERT_ANIMATION,   "Insert Animation");
      projectmenu->AppendSeparator();
      projectmenu->Append(wID_PROJECT_REFRESH,         "Refresh");
      projectmenu->Append(wID_PROJECT_RUNSPHERE,       "Run Sphere");
      projectmenu->Append(wID_PROJECT_CONFIGURESPHERE, "Configure Sphere");
      projectmenu->Append(wID_PROJECT_PACKAGE_GAME,    "Package Game");
      m_MenuBar->Insert(1, projectmenu, "Project");
    }
  } else {
    if(m_MenuBar->FindMenu("Project") != wxNOT_FOUND) {
      delete (m_MenuBar->Remove(m_MenuBar->FindMenu("Project")));
    }
  }

  if(m_PreviousChildMenuResource == m_ChildMenuResource) {
    return;
  }

  if(m_PreviousChildMenuResource != -1) {
    delete m_MenuBar->Remove(m_MenuBar->FindMenu(m_ChildMenuTitle));
  }

  if(m_ChildMenuResource == -1) {
    m_PreviousChildMenuResource = m_ChildMenuResource;
    return;
  }

  wxMenu *childmenu = new wxMenu();
  wxMenu *submenu;

  switch(m_ChildMenuResource) {
    case wID_FONT_base:
      m_ChildMenuTitle = "F&ont";
      childmenu->Append(wID_FONT_RESIZE,                "&Resize Character");
      childmenu->Append(wID_FONT_RESIZEALL,             "Resize &All Characters");
      childmenu->Append(wID_FONT_SIMPLIFY,              "&Simplify");
      childmenu->Append(wID_FONT_MAKECOLORTRANSPARENT,  "Make Color &Transparent");
      childmenu->AppendSeparator();
      childmenu->Append(wID_FONT_GENERATEGRADIENT,      "Generate &Gradient");
    break;

    case wID_IMAGE_base:
      m_ChildMenuTitle = "&Image";
      childmenu->Append(wID_IMAGE_RESIZE,               "&Resize");
      childmenu->Append(wID_IMAGE_RESCALE,              "Re&scale");
      childmenu->AppendSeparator();
      childmenu->Append(wID_IMAGE_VIEWORIGINALSIZE,     "View &Origianl Size");
    break;

    case wID_MAP_base:
      m_ChildMenuTitle = "&Map";
      childmenu->Append(wID_MAP_PROPERTIES,             "&Properties");
      childmenu->Append(wID_MAP_CHANGETILESIZE,         "&Resize Tiles");
      childmenu->Append(wID_MAP_RESCALETILESET,         "Re&scale Tiles");
      childmenu->Append(wID_MAP_RESIZEALLLAYERS,        "Resize &All Layers");
      childmenu->AppendSeparator();
      childmenu->Append(wID_MAP_EXPORTTILESET,          "&Export Tileset");
      childmenu->Append(wID_MAP_IMPORTTILESET,          "&Import Tileset");
      childmenu->Append(wID_MAP_PRUNETILESET,           "Pr&une Tileset");
    break;

    case wID_SCRIPT_base:
      m_ChildMenuTitle = "&Script";
      childmenu->Append(wID_SCRIPT_CHECKSYNTAX,         "&Check Syntax");
      childmenu->AppendSeparator();
      childmenu->Append(wID_SCRIPT_FIND,                "&Find");
      childmenu->Append(wID_SCRIPT_REPLACE,             "&Replace");
    break;

    case wID_SPRITESET_base:
      m_ChildMenuTitle = "&Spriteset";
      submenu = new wxMenu();
      submenu->Append(wID_SPRITESET_ZOOM_1X,          "x&1");
      submenu->Append(wID_SPRITESET_ZOOM_2X,          "x&2");
      submenu->Append(wID_SPRITESET_ZOOM_4X,          "x&4");
      submenu->Append(wID_SPRITESET_ZOOM_8X,          "x&8");
      childmenu->Append(wID_SPRITESET_ZOOM_, "&Zoom", submenu);
      childmenu->AppendSeparator();
      childmenu->Append(wID_SPRITESET_RESIZE,           "&Resize");
      childmenu->Append(wID_SPRITESET_FILLDELAY,        "&Fill Delay");
      childmenu->Append(wID_SPRITESET_FRAMEPROPERTIES,  "Frame &Properties");
    break;
  
    case wID_WINDOWSTYLE_base:
      m_ChildMenuTitle = "&Windowstyle";
      submenu = new wxMenu();
        submenu->Append(wID_WINDOWSTYLE_EDIT_UPPERLEFT,   "Upper Left");
        submenu->Append(wID_WINDOWSTYLE_EDIT_TOP,         "Top");
        submenu->Append(wID_WINDOWSTYLE_EDIT_UPPERRIGHT,  "Upper Right");
        submenu->Append(wID_WINDOWSTYLE_EDIT_RIGHT,       "Right");
        submenu->Append(wID_WINDOWSTYLE_EDIT_LOWERRIGHT,  "Lower Right");
        submenu->Append(wID_WINDOWSTYLE_EDIT_BOTTOM,      "Bottom");
        submenu->Append(wID_WINDOWSTYLE_EDIT_LOWERLEFT,   "Lower Left");
        submenu->Append(wID_WINDOWSTYLE_EDIT_LEFT,        "Left");
        submenu->Append(wID_WINDOWSTYLE_EDIT_BACKGROUND,  "Background");
      childmenu->Append(wID_SPRITESET_ZOOM_, "&Edit", submenu);

      submenu = new wxMenu();
        submenu->Append(wID_WINDOWSTYLE_ZOOM_1X,        "x&1");
        submenu->Append(wID_WINDOWSTYLE_ZOOM_2X,        "x&2");
        submenu->Append(wID_WINDOWSTYLE_ZOOM_4X,        "x&4");
        submenu->Append(wID_WINDOWSTYLE_ZOOM_8X,        "x&8");
      childmenu->Append(wID_SPRITESET_ZOOM_, "&Zoom", submenu);

      childmenu->Append(wID_WINDOWSTYLE_RESIZESECTION,  "&Resize Section");
      childmenu->AppendSeparator();
      childmenu->Append(wID_WINDOWSTYLE_PROPERTIES,     "&Properties");
    break;

    default:
      m_ChildMenuResource = -1;
      m_PreviousChildMenuResource = m_ChildMenuResource;
    return;
  }

  m_MenuBar->Insert((m_ProjectOpen ? 2 : 1), childmenu, m_ChildMenuTitle);
  m_PreviousChildMenuResource = m_ChildMenuResource;

}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::UpdatePaletteMenu()
{
#if 0

  HMENU menu = GetMenu()->m_hMenu;
  HMENU view_menu = GetSubMenu(menu, (m_ProjectOpen ? 2 : 1));

  // delete the old palette menu
  DeleteMenu(view_menu, 2, MF_BYPOSITION);

  // add a new one
  HMENU palette_menu = CreateMenu();
  bool empty = true;

  // get the active MDI document window
  CWnd* child = MDIGetActive();
  if (child) {
    if (GetWindowLong(child->m_hWnd, GWL_USERDATA) & WA_DOCUMENT_WINDOW) {
      CDocumentWindow* window = (CDocumentWindow*)child;

      // go through each palette and add it to the menu
      for (int i = 0; i < window->GetNumPalettes(); i++) {
        CPaletteWindow* palette = window->GetPalette(i);

        CString title;
        palette->GetWindowText(title);

        // check the palette if it's visible
        UINT visible = 0;
        if (palette->IsWindowVisible()) {
          visible = MF_CHECKED;
        }

        InsertMenu(palette_menu, i, MF_BYPOSITION | visible, PALETTE_COMMAND + i, title);
      }

      empty = (window->GetNumPalettes() == 0);
    }

  }

  // if nothing was added, make an empty palette menu
  if (empty) {
    InsertMenu(palette_menu, 0, MF_BYPOSITION, 0, "...");
    EnableMenuItem(palette_menu, 0, MF_BYPOSITION | MF_GRAYED);
  }

  InsertMenu(view_menu, 2, MF_BYPOSITION | MF_POPUP, (UINT)palette_menu, "Palettes");
#endif
}

#if 0
////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnDropFiles(HDROP hDropInfo)
{
  int     i;
  int     numfiles;
  int     length;
  LPTSTR  lpFilename;

  numfiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, NULL);
  for (i=0; i<numfiles; i++)
  {
    // here's what happens, we grab the details of the file...
    // then we transfer the details to OpenGameFile
    length = DragQueryFile(hDropInfo, i, NULL, NULL);
    length++;
    lpFilename = new char[length];

    DragQueryFile(hDropInfo, i, lpFilename, length);
    OpenGameFile(lpFilename);

    //free(lpFilename);
    delete[] lpFilename;
  }
  
  DragFinish(hDropInfo);
}
#endif

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnClose(wxCloseEvent &event)
{
  // ask if the child windows should be destroyed
  for (unsigned i = 0; i < m_DocumentWindows.size(); i++)
  {
    if (m_DocumentWindows[i]->Close()) {
      //m_DocumentWindows[i]->DestroyWindow();
    } else {
      return;
    }
  }
  m_DocumentWindows.clear();

/*todo:
  // save the command bar state
  SaveBarState(szBarState);
*/
  // close the project
  CloseProject();

/*todo:
  // store the window state
  WINDOWPLACEMENT wp;
  GetWindowPlacement(&wp);
  Configuration::Set(KEY_STARTUP, wp);

  // finally, destroy the window
  DestroyWindow();
*/

  Destroy();
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileOpen(wxCommandEvent &event)
{
  // generate list of all supported extensions
  std::set<std::string> extensions;
  for (unsigned i = 0; i < NUM_GROUP_TYPES; i++) {
    std::vector<std::string> e;
    FTL.GetFileTypeExtensions(i, e);
    for (unsigned j = 0; j < e.size(); j++) {
      extensions.insert(e[j]);
    }
  }

  // convert extension set into a string
  std::string all_filter;
  std::set<std::string>::iterator i;
  for (i = extensions.begin(); i != extensions.end(); i++) {
    all_filter += ";";
    all_filter += "*.";
    all_filter += *i;
  }

  // generate a huge filter that supports all file types!
  std::string filter;
  filter += "All Sphere Files|game.sgm" + all_filter + "|";
  filter += "Project File (game.sgm)|game.sgm|";
  for (int i = 0; i < NUM_GROUP_TYPES; i++) {
    std::vector<std::string> e;
    FTL.GetFileTypeExtensions(i, e);

    std::string type_filter;
    for (unsigned j = 0; j < e.size(); j++) {
      if (j != 0) {
        type_filter += ";";
      }
      type_filter += "*." + e[j];
    }

    filter += FTL.GetFileTypeLabel(i);
    filter += " (" + type_filter + ")|" + type_filter + "|";
  }

  filter += "All Files (*.*)|*.*||";

  char games_directory[MAX_PATH];
  GetGamesDirectory(games_directory);
  SetCurrentDirectory(games_directory);
  wxFileDialog FileDialog(this, "File Open", games_directory, "",
                          filter.c_str(), wxOPEN | wxHIDE_READONLY | wxMULTIPLE);

  // set current directory on Win98/2000
  //FileDialog.m_ofn.lpstrInitialDir = games_directory;

  // execute the dialog
  if (FileDialog.ShowModal() == wxID_OK)
  {
    wxArrayString filelist;
    FileDialog.GetPaths(filelist);
    for(unsigned i = 0; i < filelist.GetCount(); i++)
    {
      wxString thePath = filelist[i];

      if (!strcmp_ci(thePath.Right(8), "game.sgm"))
        OpenProject(thePath);
      else
        OpenGameFile(thePath);
    }

//    UpdateMenu();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileNewProject(wxCommandEvent &event)
{
  wNewProjectDialog newprojectdialog(this);
    
  if (newprojectdialog.ShowModal() == wxID_OK)
  {
    char projectname[32];
    char gametitle[32];

    strncpy(projectname, newprojectdialog.GetProjectName(), 32);
    strncpy(gametitle, newprojectdialog.GetGameTitle(), 32);

    CreateProject(projectname, gametitle);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileOpenProject(wxCommandEvent &event)
{
  char games_directory[MAX_PATH];
  GetGamesDirectory(games_directory);

  SetCurrentDirectory(games_directory);
  wxFileDialog FileDialog(
    this, "Open Project",
    games_directory,
    "game.sgm",
    "Sphere Game Files (game.sgm)|game.sgm||",
    wxOPEN | wxHIDE_READONLY
    /*TRUE,
                         "inf",
                         "game.sgm",
                         OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                         "Sphere Game Files (game.sgm)|game.sgm||"*/);

  // set current directory on Win98/2000
  //FileDialog.m_ofn.lpstrInitialDir = games_directory;

  if (FileDialog.ShowModal() == wxID_OK)
    OpenProject(FileDialog.GetPath());
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileCloseProject(wxCommandEvent &event)
{
  CloseProject();
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileOpenLastProject(wxCommandEvent &event)
{
  if (Configuration::Get(KEY_LAST_PROJECT()).length() != 0) {
    OpenProject(Configuration::Get(KEY_LAST_PROJECT()).c_str());
  }
}

////////////////////////////////////////////////////////////////////////////////

#define FILE_NEW_HANDLER(name, construct)    \
  void                                       \
  wMainWindow::OnFileNew##name(wxCommandEvent &event) \
  {                                           \
    wDocumentWindow *doc = construct;         \
    if(doc != NULL) {                         \
      m_DocumentWindows.push_back(doc);       \
    }                                         \
  }

FILE_NEW_HANDLER(Map,         new wMapWindow())
FILE_NEW_HANDLER(Script,      new wScriptWindow())
FILE_NEW_HANDLER(Spriteset,   new wSpritesetWindow())
FILE_NEW_HANDLER(Font,        new wFontWindow())
FILE_NEW_HANDLER(WindowStyle, new wWindowStyleWindow())
FILE_NEW_HANDLER(Image,       new wImageWindow())

////////////////////////////////////////////////////////////////////////////////

#define FILE_OPEN_HANDLER(name, construct)                          \
  void                                                              \
  wMainWindow::OnFileOpen##name(wxCommandEvent &event)              \
  {                                                                 \
    w##name##FileDialog Dialog(this, FDM_OPEN | FDM_MULTISELECT);   \
    if (Dialog.ShowModal() == wxID_OK) {                            \
      wxArrayString filelist;                                       \
      Dialog.GetPaths(filelist);                                    \
      for(unsigned i = 0; i < filelist.GetCount(); i++)             \
      {                                                             \
        wxString path_ = filelist[i];                               \
        const char* path = path_;                                   \
        wDocumentWindow* doc = construct;                           \
        if (doc != NULL)                                            \
          m_DocumentWindows.push_back(doc);                         \
      }                                                             \
    }                                                               \
  }


FILE_OPEN_HANDLER(Map,         new wMapWindow(path))
FILE_OPEN_HANDLER(Script,      new wScriptWindow(path))
FILE_OPEN_HANDLER(Spriteset,   new wSpritesetWindow(path))
FILE_OPEN_HANDLER(Sound,       new wSoundWindow(path))
FILE_OPEN_HANDLER(Font,        new wFontWindow(path))
FILE_OPEN_HANDLER(WindowStyle, new wWindowStyleWindow(path))
FILE_OPEN_HANDLER(Image,       new wImageWindow(path))
FILE_OPEN_HANDLER(Animation,   new wAnimationWindow(path))

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileClose(wxCommandEvent &event)
{
  if (m_DocumentWindows.size() > 0) {
    wxCloseEvent event;
    m_DocumentWindows[0]->OnClose(event);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportImageToMap(wxCommandEvent &event)
{
  // get name of image
  wImageFileDialog FileDialog(this, FDM_OPEN);
  if (FileDialog.ShowModal() != wxID_OK)
    return;

  wxString filename = FileDialog.GetPath();

  wResizeDialog resize_dialog(this, "Tile Dimensions", 16, 16);
  if (resize_dialog.ShowModal() != wxID_OK)
    return;

  // load image
  CImage32 image;
  if (image.Load(filename) == false)
  {
    ::wxMessageBox("Error: Could not load image '" + FileDialog.GetFilename() + "'");
    return;
  }

  // build map from image
  sMap map;
  if (map.BuildFromImage(image, resize_dialog.GetWidth(), resize_dialog.GetHeight()) == false)
  {
    ::wxMessageBox("Error: Could not build map from image");
    return;
  }

  char* fn = new char[strlen(filename) + 10];
  strcpy(fn, filename);

  strcpy(strrchr(fn, '.'), ".rmp");
  map.Save(fn);

  delete[] fn;

  ::wxMessageBox("Conversion successful");
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportBitmapToRWS(wxCommandEvent &event)
{
  wImageFileDialog InFileDialog(this, FDM_OPEN);
  if (InFileDialog.ShowModal() == wxID_CANCEL)
    return;

  wWindowStyleFileDialog OutFileDialog(this, FDM_SAVE);
  if (OutFileDialog.ShowModal() == wxID_CANCEL)
    return;

  sWindowStyle ws;
  if (ws.Import(InFileDialog.GetPath(), CreateRGBA(255, 0, 255, 255)) == false)
  {
    ::wxMessageBox("Can't Import file, either file is invalid \nor not a 3x3 bitmap", "Error");
    return;
  }

  if (ws.Save(OutFileDialog.GetPath()) == false)
  {
    ::wxMessageBox("Can't Save file!");
    return;
  }

  ::wxMessageBox("Import Successful!");
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportBitmapToRSS(wxCommandEvent &event)
{
  wImageFileDialog InFileDialog(this, FDM_OPEN);
  if (InFileDialog.ShowModal() != wxID_OK)
    return;

  wSpritesetFileDialog OutFileDialog(this, FDM_SAVE);
  if (OutFileDialog.ShowModal() != wxID_OK)
    return;

  wResizeDialog ResizeDialog(this, "Frame Size", 16, 32);
  ResizeDialog.SetRange(1, 4096, 1, 4096);
  if (ResizeDialog.ShowModal() != wxID_OK)
    return;

  sSpriteset sprite;
  if (sprite.Import_BMP(InFileDialog.GetPath(), ResizeDialog.GetWidth(), ResizeDialog.GetHeight(), CreateRGBA(255, 0, 255, 255)) == false)
  {
    ::wxMessageBox("Can't Import file", "Error");
    return;
  }

  if (sprite.Save(OutFileDialog.GetPath()) == false)
  {
    ::wxMessageBox("Can't Save file!");
    return;
  }

  ::wxMessageBox("Import Successful!");
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportBitmapToRTS(wxCommandEvent &event)
{
  wImageFileDialog InFileDialog(this, FDM_OPEN);
  if (InFileDialog.ShowModal() != wxID_OK)
    return;

  wTilesetFileDialog OutFileDialog(this, FDM_SAVE);
  if (OutFileDialog.ShowModal() != wxID_OK)
    return;

  wResizeDialog ResizeDialog(this, "Tile size", 16, 16);
  if (ResizeDialog.ShowModal() != wxID_OK)
    return;

  CImage32 image;
  if (!image.Load(InFileDialog.GetPath()) )
  {
    ::wxMessageBox("Can't Load image!");
    return;
  }

  sTileset tileset;
  if (!tileset.BuildFromImage(image, ResizeDialog.GetWidth(), ResizeDialog.GetHeight(), false))
  {
    ::wxMessageBox("Can't convert image!");
    return;
  }

  if (!tileset.Save(OutFileDialog.GetPath()))
  {
    ::wxMessageBox("Can't save image!");
    return;
  }

  ::wxMessageBox("Image Converted!");
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportVergeFontTemplate(wxCommandEvent &event)
{
  wImageFileDialog image_dialog(this, FDM_OPEN);
  if (image_dialog.ShowModal() != wxID_OK) {
    return;
  }

  wFontFileDialog font_dialog(this, FDM_SAVE);
  if (font_dialog.ShowModal() != wxID_OK) {
    return;
  }

  wResizeDialog resize_dialog(this, "Font character size", 8, 12);
  if (resize_dialog.ShowModal() != wxID_OK) {
    return;
  }

  sFont font;
  if (!font.ImportVergeTemplate(
        image_dialog.GetPath(),
        resize_dialog.GetWidth(),
        resize_dialog.GetHeight())) {
    ::wxMessageBox("Could not import VERGE font template", "Import VERGE font template");
    return;
  }

  if (!font.Save(font_dialog.GetPath())) {
    ::wxMessageBox("Could not save font", "Import VERGE font template");
    return;
  }

  ::wxMessageBox("Font imported successfully!", "Import VERGE font template");
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportVergeMap(wxCommandEvent &event)
{
  #define all_files "All Files (*.*)|*.*||"

  int flags = wxOPEN;
  wxFileDialog verge_map_dialog    (this, "Map file",     "", "", "VERGE Map Files (*.map)|*.map|"     all_files, flags);//TRUE, "map", NULL, flags, "VERGE Map Files (*.map)|*.map|"     all_files, this);
  wxFileDialog verge_tileset_dialog(this, "Tileset file", "", "", "VERGE Tileset Files (*.vsp)|*.vsp|" all_files, flags);//TRUE, "vsp", NULL, flags, "VERGE Tileset Files (*.vsp)|*.vsp|" all_files, this);
  wMapFileDialog map_dialog(this, FDM_SAVE);

  #undef all_files

  // load VERGE map
  if (verge_map_dialog.ShowModal() != wxID_OK) {
    return;
  }

  // load VERGE tileset
  if (verge_tileset_dialog.ShowModal() != wxID_OK) {
    return;
  }

  // import
  sMap map;
  if (!map.Import_VergeMAP(
        verge_map_dialog.GetPath(),
        verge_tileset_dialog.GetPath()
      )
  ) {
    ::wxMessageBox("Could not import Verge map");
    return;
  }

  // save
  if (map_dialog.ShowModal() == wxID_OK) {
    if (map.Save(map_dialog.GetPath())) {
      ::wxMessageBox("Import successful!");
    } else {
      ::wxMessageBox("Could not save Sphere map");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportVergeSpriteset(wxCommandEvent &event)
{
  wxFileDialog verge_spriteset_dialog(//TRUE, "chr", NULL, OFN_FILEMUSTEXIST,
    //"VERGE Spriteset Files (*.chr)|*.chr|All Files (*.*)|*.*||", this);
    this, "Spriteset", "", "", "VERGE Spriteset Files (*.chr)|*.chr|All Files (*.*)|*.*||", wxOPEN);
  wxFileDialog verge_palette_dialog(//TRUE, "pal", NULL, OFN_FILEMUSTEXIST,
    //"VERGE Palette Files (*.pal)|*.pal|All Files (*.*)|*.*||", this);
    this, "Spriteset", "", "", "VERGE Palette Files (*.pal)|*.pal|All Files (*.*)|*.*||", wxOPEN);
  wSpritesetFileDialog spriteset_dialog(this, FDM_SAVE);

  // load VERGE spriteset
  if (verge_spriteset_dialog.ShowModal() != wxID_OK) {
    return;
  }

  // load VERGE palette
  if (verge_palette_dialog.ShowModal() != wxID_OK) {
    return;
  }

  // import spriteset
  sSpriteset spriteset;
  if (!spriteset.Import_CHR(
    verge_spriteset_dialog.GetPath(),
    verge_palette_dialog.GetPath()
  )) {
    ::wxMessageBox("Could not import VERGE spriteset");
    return;
  }

  // save
  if (spriteset_dialog.ShowModal() == wxID_OK) {
    if (spriteset.Save(spriteset_dialog.GetPath())) {
      ::wxMessageBox("Import successful!");
    } else {
      ::wxMessageBox("Could not save Sphere spriteset");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportMergeRGBA(wxCommandEvent &event)
{
  wImageFileDialog rgb_dialog  (this, FDM_OPEN, "Select RGB Image");
  wImageFileDialog alpha_dialog(this, FDM_OPEN, "Select Alpha Image");
  wImageFileDialog out_dialog  (this, FDM_SAVE, "Select Output Image");

  if (rgb_dialog.DoModal()   != wxID_OK ||
      alpha_dialog.DoModal() != wxID_OK ||
      out_dialog.DoModal()   != wxID_OK) {
    return;
  }

  CImage32 rgb;
  CImage32 alpha;
  
  if (!rgb.Load(rgb_dialog.GetPath())) {
    ::wxMessageBox("Can't load image '" + rgb_dialog.GetFilename() + "'");
    return;
  }

  if (!alpha.Load(alpha_dialog.GetPath())) {
    ::wxMessageBox("Can't load image '" + alpha_dialog.GetFilename() + "'");
    return;
  }

  if (rgb.GetWidth() != alpha.GetWidth() ||
      rgb.GetHeight() != alpha.GetHeight()) {
    ::wxMessageBox("RGB and alpha images aren't same size");
    return;
  }

  CImage32 out(rgb.GetWidth(), rgb.GetHeight());
  for (int i = 0; i < rgb.GetWidth() * rgb.GetHeight(); i++) {
    RGBA* dest      = out.GetPixels() + i;
    RGBA* src_rgb   = rgb.GetPixels() + i;
    RGBA* src_alpha = alpha.GetPixels() + i;
    dest->red   = src_rgb->red;
    dest->green = src_rgb->green;
    dest->blue  = src_rgb->blue;
    dest->alpha = (src_alpha->red + src_alpha->green + src_alpha->blue) / 3;
  }

  if (!out.Save(out_dialog.GetPath())) {
    ::wxMessageBox("Can't save image '" + out_dialog.GetFilename() + "'");
    return;
  }

  ::wxMessageBox("Image merged successfully");
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileImportWindowsFont(wxCommandEvent &event)
{
/*todo:
  // windows font dialog
  CFontDialog font_dialog(NULL, CF_FORCEFONTEXIST | CF_EFFECTS | CF_SCREENFONTS);
  if (font_dialog.DoModal() != wxID_OK) {
    return;
  }
  COLORREF color = font_dialog.GetColor();

  // get the font from the dialog
  LOGFONT lf;
  font_dialog.GetCurrentFont(&lf);

  // grab the font
  HDC dc = CreateCompatibleDC(NULL);
  HFONT font = CreateFontIndirect(&lf);
  HFONT old_font = (HFONT)SelectObject(dc, font);

  sFont sphere_font(256);
  
  for (int i = 0; i < 256; i++) {
    char c = (char)i;
    SIZE size;
    GetTextExtentPoint32(dc, &c, 1, &size);

    // grab the character
    CDIBSection* dib = new CDIBSection(size.cx, size.cy, 32);
    memset(dib->GetPixels(), 255, size.cx * size.cy * 4); // opaque white
    RECT rect = { 0, 0, size.cx, size.cy };

    SetTextColor(dib->GetDC(), 0);  // black
    SetBkMode(dib->GetDC(), TRANSPARENT);
    HFONT old_font = (HFONT)SelectObject(dib->GetDC(), font);
    DrawText(dib->GetDC(), &c, 1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
    SelectObject(dib->GetDC(), old_font);

    // put it into the font
    int width = size.cx + 1;
    sphere_font.GetCharacter(i).Resize(width, size.cy);
    memset(sphere_font.GetCharacter(i).GetPixels(), 0, width * size.cy * 4);

    BGRA* src = (BGRA*)dib->GetPixels();
    RGBA* dst = sphere_font.GetCharacter(i).GetPixels();
    for (int iy = 0; iy < size.cy; iy++) {
      for (int ix = 0; ix < size.cx; ix++) {
        dst[iy * width + ix].red   = GetRValue(color);
        dst[iy * width + ix].green = GetGValue(color);
        dst[iy * width + ix].blue  = GetBValue(color);
        dst[iy * width + ix].alpha = 255 - (src->red + src->green + src->blue) / 3;
        src++;
      }
    }

    delete dib;
  }

  SelectObject(dc, old_font);
  DeleteObject(font);
  DeleteDC(dc);

  // save the Sphere font
  CFontFileDialog file_dialog(FDM_SAVE, "Select Output Font");
  if (file_dialog.DoModal() != IDOK) {
    return;
  }

  if (!sphere_font.Save(file_dialog.GetPathName())) {
    MessageBox("Could not save font");
  }
*/
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileSaveAll(wxCommandEvent &event)
{
  for (unsigned i = 0; i < m_DocumentWindows.size(); i++) {
    wDocumentWindow* dw = m_DocumentWindows[i];
    if (dw->IsSaveable()) {
      wSaveableDocumentWindow* sdw = (wSaveableDocumentWindow*)dw;
      sdw->Save();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnFileOptions(wxCommandEvent &event)
{
/*todo:
  COptionsDialog().DoModal();
*/
}


////////////////////////////////////////////////////////////////////////////////

#define PROJECT_INSERT_HANDLER(type, group_type)                        \
void                                                                    \
wMainWindow::OnProjectInsert##type(wxCommandEvent &event)               \
{                                                                       \
  w##type##FileDialog dialog(this, FDM_OPEN | FDM_MAYNOTEXIST);         \
  InsertProjectFile(&dialog, group_type);                               \
                                                                        \
  /* save the project and update the view                     */        \
  /* (in case user changed filename extensions in the dialog) */        \
  m_Project.RefreshItems();                                             \
  m_ProjectWindow->Update();                                            \
}

PROJECT_INSERT_HANDLER(Map,         GT_MAPS)
PROJECT_INSERT_HANDLER(Spriteset,   GT_SPRITESETS)
PROJECT_INSERT_HANDLER(Script,      GT_SCRIPTS)
PROJECT_INSERT_HANDLER(Sound,       GT_SOUNDS)
PROJECT_INSERT_HANDLER(Font,        GT_FONTS)
PROJECT_INSERT_HANDLER(WindowStyle, GT_WINDOWSTYLES)
PROJECT_INSERT_HANDLER(Image,       GT_IMAGES)
PROJECT_INSERT_HANDLER(Animation,   GT_ANIMATIONS)

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnProjectRefresh(wxCommandEvent &event)
{
  if (m_ProjectWindow) {
    m_ProjectWindow->Update();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnProjectRunSphere(wxCommandEvent &event)
{
  /*todo:add save all check*/
  char szCommandLine[MAX_PATH + 80];
  strcpy(szCommandLine, g_SphereDirectory.c_str());
#ifdef WIN32
  strcat(szCommandLine, "\\engine.exe -game ");
#else
  strcat(szCommandLine, "/engine -game ");
#endif
  strcat(szCommandLine, "\"");
  strcat(szCommandLine, m_Project.GetGameSubDirectory());
  strcat(szCommandLine, "\"");

/*
  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
*/  
  char sphere_directory[MAX_PATH];
  strcpy(sphere_directory, g_SphereDirectory.c_str());

/*
  BOOL retval = CreateProcess(
    NULL,                  // lpApplicationName
    szCommandLine,         // lpCommandLine
    NULL,                  // lpProcessAttributes
    NULL,                  // lpThreadAttributes
    FALSE,                 // bInheritHandles
    0,                     // dwCreationFlags
    NULL,                  // lpEnvironment
    sphere_directory,      // lpCurrentDirectory
    &si,                   // lpStartupInfo
    &pi);                  // lpProcessInformation
  if (retval == FALSE)
*/
  if(wxExecute(szCommandLine) == -1) {
    ::wxMessageBox("Error: Could not execute Sphere engine");
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnProjectConfigureSphere(wxCommandEvent &event)
{
  char sphere_directory[MAX_PATH];
  strcpy(sphere_directory, g_SphereDirectory.c_str());

  char szCommandLine[MAX_PATH + 80];
  strcpy(szCommandLine, sphere_directory);
  strcat(szCommandLine, "\\config.exe");
/*
  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  BOOL retval = CreateProcess(
    NULL,                  // lpApplicationName
    szCommandLine,         // lpCommandLine
    NULL,                  // lpProcessAttributes
    NULL,                  // lpThreadAttributes
    FALSE,                 // bInheritHandles
    0,                     // dwCreationFlags
    NULL,                  // lpEnvironment
    sphere_directory,      // lpCurrentDirectory
    &si,                   // lpStartupInfo
    &pi);                  // lpProcessInformation
  if (retval == FALSE)
*/
  if(wxExecute(szCommandLine) == -1) {
    ::wxMessageBox("Error: Could not configure Sphere");
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnProjectPackageGame(wxCommandEvent &event)
{
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);
  SetCurrentDirectory(m_Project.GetDirectory());

  // go into game directory and add all files
  std::list<std::string> files;
  EnumerateFiles("*.*", "", files);

  CPackage package;
  std::list<std::string>::iterator i;
  for (i = files.begin(); i != files.end(); i++) {
    package.AddFile(i->c_str());
  }

  // TODO:  show a wait dialog (no cancel)

  wPackageFileDialog dialog(this, FDM_SAVE);
  /*todo:dialog.m_ofn.Flags |= OFN_NOCHANGEDIR;*/

  if (dialog.ShowModal() == wxID_OK) {
    // write the package!
    if (!package.Write(dialog.GetPath())) {
      ::wxMessageBox("Package creation failed", "Package Game");
    } else {
      ::wxMessageBox("Package creation succeeded!", "Package Game");
    }
  }

  SetCurrentDirectory(old_directory);
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnWindowCloseAll(wxCommandEvent &event)
{
  for (unsigned i = 0; i < m_DocumentWindows.size(); i++) {
    wDocumentWindow* dw = m_DocumentWindows[i];
    if (dw->Close() == false) {
      break;
    }

    //dw->Destroy();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnHelpContents(wxCommandEvent &event)
{
//  if (HtmlHelp(m_hWnd, "docs/editor.chm", HH_DISPLAY_TOC, 0) == NULL) {
    ::wxMessageBox("Error: Could not open help file", "Help Contents");
//  }
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::OnHelpAbout(wxCommandEvent &event)
{
  char message[1024];
  sprintf(message,
    "Sphere Development Environment\n"
    SPHERE_VERSION "\n"
    "\n"
    "Chad Austin (c) 1999-2001\n"
    "Additional code by Darklich\n"
    "Icons by Khadgar\n"
    "\n"
    "\n"
    __DATE__ "\n"
    __TIME__ "\n"
    "\n"
    "JavaScript: %s\n"
    "zlib: %s\n"
    "libmng: DLL %s - header %s\n",

    JS_GetImplementationVersion(),
    zlibVersion(),
    mng_version_text(), MNG_VERSION_TEXT);

  ::wxMessageBox(message, "About");
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::SetChildMenu(int childmenu, wxEvtHandler *receiver)
{
  ClearChildMenu();
  m_MenuReceiver = receiver;
  m_ChildMenuResource = childmenu;
  UpdateMenu();
}

////////////////////////////////////////////////////////////////////////////////

void
wMainWindow::ClearChildMenu()
{
  m_ChildMenuResource = -1;
  m_MenuReceiver = NULL;
  UpdateMenu();
}

////////////////////////////////////////////////////////////////////////////////

//

bool
wMainWindow::ProcessEvent(wxEvent &event)
{
  bool retval;
  if(m_InsideEventProcess > 0) {
    return false;
  }
  if(wxMDIParentFrame::ProcessEvent(event)) {
    return true;
  }
  if(m_MenuReceiver != NULL) {
    m_InsideEventProcess++;
    retval = m_MenuReceiver->ProcessEvent(event);
    m_InsideEventProcess--;
    return retval;
  }

  return false;
}

#if 0
todo:

////////////////////////////////////////////////////////////////////////////////

afx_msg BOOL
CMainWindow::OnNeedText(UINT /*id*/, NMHDR* nmhdr, LRESULT* result)
{
  TOOLTIPTEXT* ttt = (TOOLTIPTEXT*)nmhdr;
  UINT id = nmhdr->idFrom;
  if (ttt->uFlags & TTF_IDISHWND) {
    id = ::GetDlgCtrlID((HWND)id);
  }

  switch (id) {
  case ID_FILE_NEW_PROJECT:  ttt->lpszText = "New Sphere Project"; break;
  case ID_PROJECT_RUNSPHERE: ttt->lpszText = "Run Sphere";         break;
  default:                   ttt->lpszText = "";                   break;
  }

  *result = 0;
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnUpdateOpenLastProject(CCmdUI* cmdui)
{
  if (Configuration::Get(KEY_LAST_PROJECT).length() == 0) {
    cmdui->Enable(FALSE);
  } else {
    cmdui->Enable(TRUE);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnUpdateProjectCommand(CCmdUI* cmdui)
{
  if (m_ProjectOpen)
    cmdui->Enable(TRUE);
  else
    cmdui->Enable(FALSE);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnUpdateSaveCommand(CCmdUI* cmdui)
{
  CWnd* pWindow = MDIGetActive();
  if (pWindow == NULL)
    cmdui->Enable(FALSE);
  else
  {
    if (GetWindowLong(pWindow->m_hWnd, GWL_USERDATA) & WA_SAVEABLE)
      cmdui->Enable(TRUE);
    else
      cmdui->Enable(FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnUpdateSaveAllCommand(CCmdUI* cmdui)
{
  bool can_save = false;

  // if any window is modified or unsaved, enable it!
  for (int i = 0; i < m_DocumentWindows.size(); i++) {
    CDocumentWindow* dw = m_DocumentWindows[i];
    if (dw->IsSaveable()) {
      CSaveableDocumentWindow* sdw = (CSaveableDocumentWindow*)dw;
      if (sdw->IsSaved() == false || sdw->IsModified() == true) {
        can_save = true;
        break;
      }
    }
  }

  cmdui->Enable(can_save);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnUpdateWindowCloseAll(CCmdUI* cmdui)
{
  cmdui->Enable(m_DocumentWindows.size() > 0);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnInsertProjectFile(WPARAM wparam, LPARAM lparam)
{
  // local functions
  struct Local {
    static inline bool extension_compare(const char* path, const char* extension) {
      int path_length = strlen(path);
      int ext_length  = strlen(extension);
      return (
        path_length >= ext_length &&
        strcmp(path + path_length - ext_length, extension) == 0
      );
    }
  };

  const char* path = (const char*)lparam;

  // figure out what filetype the file is
  for (int i = 0; i < NUM_GROUP_TYPES; i++ ) {

    std::vector<std::string> extensions;
    FTL.GetFileTypeExtensions(i, extensions);

    for (int j = 0; j < extensions.size(); j++) {
      if (Local::extension_compare(path, extensions[j].c_str())) {
        InsertProjectFile(NULL, i, path);
        return;
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnRefreshProject(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
  m_Project.RefreshItems();
  if (m_ProjectWindow) {
    m_ProjectWindow->Update();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg LRESULT
CMainWindow::OnDocumentWindowClosing(WPARAM wparam, LPARAM lparam)
{
  CDocumentWindow* window = (CDocumentWindow*)lparam;

  // remove window from list
  for (int i = 0; i < m_DocumentWindows.size(); i++)
    if (m_DocumentWindows[i] == window)
    {
      m_DocumentWindows.erase(m_DocumentWindows.begin() + i);
      return 0;
    }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg LRESULT
CMainWindow::OnSetChildMenu(WPARAM wparam, LPARAM lparam)
{
  m_ChildMenuResource = wparam;
  UpdateMenu();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg LRESULT
CMainWindow::OnClearChildMenu(WPARAM wparam, LPARAM lparam)
{
  m_ChildMenuResource = -1;
  UpdateMenu();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg LRESULT
CMainWindow::OnUpdatePaletteMenu(WPARAM wparam, LPARAM lparam)
{
  UpdatePaletteMenu();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnViewPalette(UINT id)
{
  int palette_num = id - PALETTE_COMMAND;

  // toggle the visibility of the palette

  CWnd* child = MDIGetActive();
  if (child) {
    if (GetWindowLong(child->m_hWnd, GWL_USERDATA) & WA_DOCUMENT_WINDOW) {
      CDocumentWindow* window = (CDocumentWindow*)child;

      // find the palette to toggle
      CPaletteWindow* palette = window->GetPalette(palette_num);
      palette->ShowWindow(palette->IsWindowVisible() ? SW_HIDE : SW_SHOW);
    }
  }

  UpdatePaletteMenu();
}
#endif
////////////////////////////////////////////////////////////////////////////////
