// disable 'identifier too long' warning
#pragma warning(disable : 4786)

// standard
#include <set>

// system
#include <afxdlgs.h>

// core
#include "Editor.hpp"
#include "WindowCommands.hpp"
#include "FileSystem.hpp"
#include "Configuration.hpp"
#include "Keys.hpp"
#include "Package.hpp"
#include "resource.h"

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
#include "BrowseWindow.hpp"

// dialogs
#include "NewProjectDialog.hpp"
#include "GameSettingsDialog.hpp"
#include "FileDialogs.hpp"
#include "ResizeDialog.hpp"
#include "StringDialog.hpp"
#include "FontGradientDialog.hpp"

// common
#include "../common/sphere_version.h"
#include "../common/Map.hpp"
#include "../common/strcmp_ci.hpp"
#include "../common/system.hpp"

// libraries
// libraries are down here because of symbol conflicts with other headers
#include <jsapi.h>
#include <zlib.h>
#include <libmng.h>
#include <audiere.h>
#include <corona.h>


// base for palette menu items
const int PALETTE_COMMAND = 17133;
const int NUM_PALETTES    = 100;


const char szBarState[] = "SDE_BarState";


BEGIN_MESSAGE_MAP(CMainWindow, CMDIFrameWnd)

  ON_WM_DROPFILES()
  ON_WM_CLOSE()

  // generic file open
  ON_COMMAND(ID_FILE_OPEN, OnFileOpen)

  // project
  ON_COMMAND(ID_FILE_NEW_PROJECT,     OnFileNewProject)
  ON_COMMAND(ID_FILE_OPEN_PROJECT,    OnFileOpenProject)
  ON_COMMAND(ID_FILE_CLOSEPROJECT,    OnFileCloseProject)
  ON_COMMAND(ID_FILE_OPENLASTPROJECT, OnFileOpenLastProject)

  // file | new
  ON_COMMAND(ID_FILE_NEW_MAP,         OnFileNewMap)
  ON_COMMAND(ID_FILE_NEW_SPRITESET,   OnFileNewSpriteset)
  ON_COMMAND(ID_FILE_NEW_SCRIPT,      OnFileNewScript)
  ON_COMMAND(ID_FILE_NEW_FONT,        OnFileNewFont)
  ON_COMMAND(ID_FILE_NEW_WINDOWSTYLE, OnFileNewWindowStyle)
  ON_COMMAND(ID_FILE_NEW_IMAGE,       OnFileNewImage)

  // file | open
  ON_COMMAND(ID_FILE_OPEN_MAP,         OnFileOpenMap)
  ON_COMMAND(ID_FILE_OPEN_SPRITESET,   OnFileOpenSpriteset)
  ON_COMMAND(ID_FILE_OPEN_SCRIPT,      OnFileOpenScript)
  ON_COMMAND(ID_FILE_OPEN_SOUND,       OnFileOpenSound)
  ON_COMMAND(ID_FILE_OPEN_FONT,        OnFileOpenFont)
  ON_COMMAND(ID_FILE_OPEN_WINDOWSTYLE, OnFileOpenWindowStyle)
  ON_COMMAND(ID_FILE_OPEN_IMAGE,       OnFileOpenImage)
  ON_COMMAND(ID_FILE_OPEN_ANIMATION,   OnFileOpenAnimation)
  ON_COMMAND(ID_FILE_OPEN_TILESET,     OnFileOpenTileset)

  // file | import
  ON_COMMAND(ID_FILE_IMPORT_IMAGETOMAPTILESET, OnFileImportImageToMap)
  ON_COMMAND(ID_FILE_IMPORT_BITMAPTORWS,       OnFileImportBitmapToRWS)
  ON_COMMAND(ID_FILE_IMPORT_BITMAPTORSS,       OnFileImportBitmapToRSS)
  ON_COMMAND(ID_FILE_IMPORT_BITMAPTORTS,       OnFileImportBitmapToRTS)
  ON_COMMAND(ID_FILE_IMPORT_VERGEFONTTEMPLATE, OnFileImportVergeFontTemplate)
  ON_COMMAND(ID_FILE_IMPORT_VERGEMAP,          OnFileImportVergeMap)
  ON_COMMAND(ID_FILE_IMPORT_VERGESPRITESET,    OnFileImportVergeSpriteset)
  ON_COMMAND(ID_FILE_IMPORT_MERGE_RGBA,        OnFileImportMergeRGBA)
  ON_COMMAND(ID_FILE_IMPORT_WINDOWSFONT,       OnFileImportWindowsFont)
  ON_COMMAND(ID_FILE_IMPORT_RM2KCHARSETTORSS,  OnFileImportRM2KCharsetToRSS)
  ON_COMMAND(ID_FILE_IMPORT_RM2KCHIPSETTORTS,  OnFileImportRM2KChipsetToRTS)

  ON_COMMAND(ID_FILE_SAVEALL, OnFileSaveAll)

  ON_COMMAND(ID_FILE_EXIT,    OnClose)

  ON_COMMAND(ID_FILE_BROWSE,  OnFileBrowse)

  // insert
  ON_COMMAND(ID_PROJECT_INSERT_MAP,         OnProjectInsertMap)
  ON_COMMAND(ID_PROJECT_INSERT_SPRITESET,   OnProjectInsertSpriteset)
  ON_COMMAND(ID_PROJECT_INSERT_SCRIPT,      OnProjectInsertScript)
  ON_COMMAND(ID_PROJECT_INSERT_SOUND,       OnProjectInsertSound)
  ON_COMMAND(ID_PROJECT_INSERT_FONT,        OnProjectInsertFont)
  ON_COMMAND(ID_PROJECT_INSERT_WINDOWSTYLE, OnProjectInsertWindowStyle)
  ON_COMMAND(ID_PROJECT_INSERT_IMAGE,       OnProjectInsertImage)
  ON_COMMAND(ID_PROJECT_INSERT_ANIMATION,   OnProjectInsertAnimation)

  ON_COMMAND(ID_PROJECT_REFRESH,         OnProjectRefresh)
  ON_COMMAND(ID_PROJECT_RUNSPHERE,       OnProjectRunSphere)
  ON_COMMAND(ID_PROJECT_CONFIGURESPHERE, OnProjectConfigureSphere)
  ON_COMMAND(ID_PROJECT_PACKAGE_GAME,    OnProjectPackageGame)
  ON_COMMAND(ID_PROJECT_MAKE_INSTALLER,  OnProjectMakeInstaller)

	ON_COMMAND(ID_VIEW_PROJECT,            OnViewProject)

  ON_COMMAND(ID_WINDOW_CLOSEALL, OnWindowCloseAll)

  ON_COMMAND(ID_HELP_SPHERESITE,         OnHelpSphereSite)
  ON_COMMAND(ID_HELP_SPHEREFAQ,          OnHelpSphereFAQ)
  ON_COMMAND(ID_HELP_AEGISKNIGHTSSITE,   OnHelpAegisKnightsSite)
  ON_COMMAND(ID_HELP_FLIKSSITE,          OnHelpFliksSite)
  ON_COMMAND(ID_HELP_LOCALDOCUMENTATION, OnHelpLocalDocumentation)
  ON_COMMAND(ID_HELP_ABOUT,              OnHelpAbout)

  ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnNeedText)

  ON_UPDATE_COMMAND_UI(ID_FILE_OPENLASTPROJECT, OnUpdateOpenLastProject)
  ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEPROJECT, OnUpdateProjectCommand)

  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE,       OnUpdateSaveCommand)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVEAS,     OnUpdateSaveCommand)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVECOPYAS, OnUpdateSaveCommand)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVEALL,    OnUpdateSaveAllCommand)

  ON_UPDATE_COMMAND_UI(ID_PROJECT_RUNSPHERE, OnUpdateProjectCommand)
  ON_UPDATE_COMMAND_UI(ID_WINDOW_CLOSEALL,   OnUpdateWindowCloseAll)
  ON_UPDATE_COMMAND_UI(ID_VIEW_PALETTES,     OnUpdatePaletteMenu)
  ON_UPDATE_COMMAND_UI(PALETTE_COMMAND,	     OnUpdatePaletteMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROJECT,	     OnUpdateViewProject)

  // project window message
  ON_MESSAGE(WM_INSERT_PROJECT_FILE, OnInsertProjectFile)
  ON_MESSAGE(WM_REFRESH_PROJECT,     OnRefreshProject)
  
  // document window messages
  ON_MESSAGE(WM_DW_CLOSING,          OnDocumentWindowClosing)
  ON_MESSAGE(WM_SET_CHILD_MENU,      OnSetChildMenu)
  ON_MESSAGE(WM_CLEAR_CHILD_MENU,    OnClearChildMenu)  
  ON_COMMAND_RANGE(PALETTE_COMMAND, PALETTE_COMMAND + NUM_PALETTES, OnViewPalette)

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////

CMainWindow::CMainWindow()
: m_ProjectOpen(false)
, m_ProjectWindow(NULL)
, m_ChildMenuResource(-1)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL
CMainWindow::Create()
{
  // create the window
  CMDIFrameWnd::Create(
    AfxRegisterWndClass(0, NULL, NULL, LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_SDE))),
    "Sphere " SPHERE_VERSION " Development Environment",
    WS_OVERLAPPEDWINDOW,
    rectDefault,
    NULL,
    MAKEINTRESOURCE(IDR_MAIN));

  LoadAccelTable(MAKEINTRESOURCE(IDR_ACCELERATOR));

  // create the toolbar
  m_ToolBar.CreateEx(
    this,
    TBSTYLE_FLAT,
    WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC | CBRS_TOP | CBRS_GRIPPER | CBRS_FLYBY | CBRS_TOOLTIPS);
  m_ToolBar.SetWindowText("Main");
  m_ToolBar.LoadToolBar(IDR_TOOLBAR);
  m_ToolBar.EnableDocking(CBRS_ALIGN_ANY);

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
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void
CMainWindow::CreateProject(const char* projectname, const char* gametitle)
{
  CloseProject();

  char games_directory[MAX_PATH];
  GetGamesDirectory(games_directory);

  // this may fail, but we don't care
  CreateDirectory(games_directory, NULL);
  
  if (!m_Project.Create(games_directory, projectname))
  {
    MessageBox("Error: Could not create project");
    return;
  }

  m_Project.SetGameTitle(gametitle);
  m_ProjectOpen = true;
  m_ProjectWindow = new CProjectWindow(this, &m_Project);
  m_ProjectWindow->Create();

  UpdateMenu();
}

////////////////////////////////////////////////////////////////////////////////

void
CMainWindow::OpenProject(const char* filename)
{
  CloseProject();
  
  if (m_Project.Open(filename) == false)
  {
    MessageBox("Could not open project");
    return;
  }

  Configuration::Set(KEY_LAST_PROJECT, filename);

  m_ProjectOpen = true;
  m_ProjectWindow = new CProjectWindow(this, &m_Project);
  m_ProjectWindow->Create();

  UpdateMenu();
}

////////////////////////////////////////////////////////////////////////////////

void
CMainWindow::CloseProject()
{
  if (m_ProjectOpen)
  {
    m_ProjectOpen = false;

    if (m_ProjectWindow)
    {
#ifdef USE_SIZECBAR
			m_ProjectWindow->ShowPalette(false);
      m_ProjectWindow->CreateBar(false);
#endif
			m_ProjectWindow->DestroyWindow();
      m_ProjectWindow = NULL;
    }

    UpdateMenu();
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMainWindow::OpenGameSettings()
{
  CGameSettingsDialog(&m_Project).DoModal();
}

////////////////////////////////////////////////////////////////////////////////

void
CMainWindow::OpenGameFile(const char* filename)
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
    
    for (int k = 0; k < extensions.size(); k++) {
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
CMainWindow::OpenDocumentWindow(int grouptype, const char* filename)
{
  // if a document window has the same filename, just give it focus
  for (int i = 0; i < m_DocumentWindows.size(); i++) {
    if (strcmp(m_DocumentWindows[i]->GetFilename(), filename) == 0) {
      m_DocumentWindows[i]->SetFocus();
      return;
    }
  }

  // actually open the window now
  CDocumentWindow* window = NULL;
  switch (grouptype)
  {
    case GT_MAPS:         window = new CMapWindow(filename);         break;
    case GT_SPRITESETS:   window = new CSpritesetWindow(filename);   break;
    case GT_SCRIPTS:      window = new CScriptWindow(filename);      break;
    case GT_SOUNDS:       window = new CSoundWindow(filename);       break;
    case GT_FONTS:        window = new CFontWindow(filename);        break;
    case GT_WINDOWSTYLES: window = new CWindowStyleWindow(filename); break;
    case GT_IMAGES:       window = new CImageWindow(filename);       break;
    case GT_ANIMATIONS:   window = new CAnimationWindow(filename);   break;
    case GT_TILESETS:     window = new CMapWindow(NULL, filename);   break;
  }

  if (window) {
    m_DocumentWindows.push_back(window);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
CMainWindow::AddToDirectory(const char* pathname, const char* sub_directory)
{
  // if file isn't already in the subdirectory
  if (!CheckDirectory(pathname, sub_directory))
  {
    // ask the user if we can copy it
    char message[1024];
    sprintf(message, "The file must be copied into the game '%s' directory.  Is this okay?", sub_directory);
    if (MessageBox(message, NULL, MB_YESNO) == IDNO)
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
      if (MessageBox("File appears to already exist, overwrite?", NULL, MB_YESNO) == IDNO)
        return false;

      if (CopyFile(pathname, szDestination, FALSE) == FALSE)
      {
        MessageBox("Error: Could not copy file");
        return false;
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
CMainWindow::CheckDirectory(const char* filename, const char* sub_directory)
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
  strlwr(szDirectory);
  strlwr(szProjectDirectory);
  return (strcmp(szDirectory, szProjectDirectory) == 0);
}

////////////////////////////////////////////////////////////////////////////////

void
CMainWindow::InsertProjectFile(CFileDialog* file_dialog, int grouptype, const char* predefined_path)
{
  // determine filetype directory
  char path[MAX_PATH];
  sprintf(path, "%s\\%s", m_Project.GetDirectory(), m_Project.GetGroupDirectory(grouptype));
  if (!PathExists(path))
  {
    char szMessage[80];
    sprintf(szMessage, "'%s' subdirectory does not exist, create?", m_Project.GetGroupDirectory(grouptype));
    if (MessageBox(szMessage, NULL, MB_YESNO) == IDNO)
      return;

    // create the directory
    if (!CreateDirectory(path, NULL))
    {
      char szMessage[MAX_PATH + 80];
      sprintf(szMessage, "Error: Could not create directory '%s'", path);
      MessageBox(szMessage);
      return;
    }
  }

  CString sPathName;

  if (file_dialog) {
    // start the file dialog in the correct directory
    SetCurrentDirectory(path);
    if (file_dialog->DoModal() != IDOK)
      return;

    // we've got a full file path
    sPathName = file_dialog->GetPathName();
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
      MessageBox("File is already in project");
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
    AddToDirectory(sPathName, CProject::GetGroupDirectory(grouptype));
  }
}

////////////////////////////////////////////////////////////////////////////////

void
CMainWindow::GetGamesDirectory(char games_directory[MAX_PATH])
{
  strcpy(games_directory, g_SphereDirectory.c_str());
  if (games_directory[strlen(games_directory) - 1] != '\\')
    strcat(games_directory, "\\");
  strcat(games_directory, "games");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::UpdateMenu()
{
  int iWindowMenu = 2;

  // destroy the old menu
  // FIXME: how to delete menu right
  // note: gives 0 byte memory leak if not enabled but...
  //       gives win9x MDI windowing problems if enabled...
  //  if (GetMenu())
  //    GetMenu()->DestroyMenu();

  // create the new menu
  HINSTANCE hInstance = AfxGetApp()->m_hInstance;
  HMENU hNewMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAIN));

  // if a project is open, add the project menu
  if (m_ProjectOpen)
  {
    HMENU hProjectMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_PROJECT));

    char szPopupTitle[80];
    GetMenuString(hProjectMenu, 0, szPopupTitle, 80, MF_BYPOSITION);

    InsertMenu(hNewMenu,
               1,
               MF_POPUP | MF_BYPOSITION | MF_STRING,
               (UINT_PTR)GetSubMenu(hProjectMenu, 0),
               szPopupTitle);

    iWindowMenu++;
  }

  // if a child menu is set, add it
  if (m_ChildMenuResource != -1)
  {
    HMENU hChildMenu = LoadMenu(hInstance, MAKEINTRESOURCE(m_ChildMenuResource));

    char szPopupTitle[80];
    GetMenuString(hChildMenu, 0, szPopupTitle, 80, MF_BYPOSITION);
    
    InsertMenu(hNewMenu,
               iWindowMenu,
               MF_POPUP | MF_BYPOSITION | MF_STRING,
               (UINT_PTR)GetSubMenu(hChildMenu, 0),
               szPopupTitle);

    iWindowMenu++;
  }

  // set the new menu
  CMenu* pNewMenu = CMenu::FromHandle(hNewMenu);
  MDISetMenu(pNewMenu, pNewMenu->GetSubMenu(iWindowMenu))->DestroyMenu();
  DrawMenuBar();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnDropFiles(HDROP hDropInfo)
{
  int     i;
  int     numfiles;
  int     length;
  LPTSTR  lpFilename;

  numfiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, NULL);
  for (i = 0; i < numfiles; i++)
  {
    // here's what happens, we grab the details of the file...
    // then we transfer the details to OpenGameFile
    length = DragQueryFile(hDropInfo, i, NULL, NULL);
    length++;
    lpFilename = new char[length];
    if (lpFilename == NULL)
      return;

    DragQueryFile(hDropInfo, i, lpFilename, length);
    OpenGameFile(lpFilename);

    delete[] lpFilename;
  }
  
  DragFinish(hDropInfo);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnClose()
{
  // ask if the child windows should be destroyed
  for (int i = 0; i < m_DocumentWindows.size(); i++)
  {
    if (m_DocumentWindows[i]->Close())
      m_DocumentWindows[i]->DestroyWindow();
    else
      return;
  }
  m_DocumentWindows.clear();

  // save the command bar state
#ifndef USE_SIZECBAR
  SaveBarState(szBarState);
#endif

  // close the project
  CloseProject();

  // store the window state
  WINDOWPLACEMENT wp;
  GetWindowPlacement(&wp);
  Configuration::Set(KEY_STARTUP, wp);

  // finally, destroy the window
  DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////

std::string GenerateSupportedExtensionsFilter() {
  // generate list of all supported extensions
  std::set<std::string> extensions;
  for (int i = 0; i < NUM_GROUP_TYPES; i++) {
    std::vector<std::string> e;
    FTL.GetFileTypeExtensions(i, e);
    for (int j = 0; j < e.size(); j++) {
      std::string poop = e[j];
      extensions.insert(poop);
    }
  }

  // convert extension set into a string
  std::string all_filter;
  std::set<std::string>::iterator j;
  for (j = extensions.begin(); j != extensions.end(); j++) {
    all_filter += ";";
    all_filter += "*.";
    all_filter += *j;
  }

  // generate a huge filter that supports all file types!
  std::string filter;
  filter += "All Sphere Files|game.sgm" + all_filter + "|";
  filter += "Project File (game.sgm)|game.sgm|";
  for (int i = 0; i < NUM_GROUP_TYPES; i++) {
    std::vector<std::string> e;
    FTL.GetFileTypeExtensions(i, e);

    std::string type_filter;
    for (int j = 0; j < e.size(); j++) {
      if (j != 0) {
        type_filter += ";";
      }
      type_filter += "*." + e[j];
    }

    filter += FTL.GetFileTypeLabel(i);
    filter += " (" + type_filter + ")|" + type_filter + "|";
  }

  filter += "All Files (*.*)|*.*||";
  return filter;
}

afx_msg void
CMainWindow::OnFileOpen()
{
  std::string filter = GenerateSupportedExtensionsFilter();

  char games_directory[MAX_PATH];
  GetGamesDirectory(games_directory);
  SetCurrentDirectory(games_directory);
  CFileDialog FileDialog(
    TRUE,
    "",
    NULL,
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT,
    filter.c_str());

  // set current directory on Win98/2000
  FileDialog.m_ofn.lpstrInitialDir = games_directory;

  // execute the dialog
  if (FileDialog.DoModal() == IDOK)
  {
    POSITION pos = FileDialog.GetStartPosition();

    while (pos != NULL)
    {
      CString thePath = FileDialog.GetNextPathName(pos);

      if (!strcmp_ci(thePath.Right(8), "game.sgm"))
        OpenProject(thePath);
      else
        OpenGameFile(thePath);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileNewProject()
{
  CNewProjectDialog newprojectdialog(this);
    
  if (newprojectdialog.DoModal() == IDOK)
  {
    char projectname[32];
    char gametitle[32];

    strncpy(projectname, newprojectdialog.GetProjectName(), 32);
    strncpy(gametitle, newprojectdialog.GetGameTitle(), 32);

    CreateProject(projectname, gametitle);

    char path_name[MAX_PATH];
    GetGamesDirectory(path_name);
    sprintf (path_name, "%s\\%s\\game.sgm", path_name, gametitle);

    Configuration::Set(KEY_LAST_PROJECT, path_name);
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileOpenProject()
{
  char games_directory[MAX_PATH];
  GetGamesDirectory(games_directory);

  SetCurrentDirectory(games_directory);
  CFileDialog FileDialog(TRUE,
                         "inf",
                         "game.sgm",
                         OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                         "Sphere Game Files (game.sgm)|game.sgm||");

  // set current directory on Win98/2000
  FileDialog.m_ofn.lpstrInitialDir = games_directory;

  if (FileDialog.DoModal() == IDOK) {
    OpenProject(FileDialog.GetPathName());
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileCloseProject()
{
  CloseProject();
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileOpenLastProject()
{
  if (Configuration::Get(KEY_LAST_PROJECT).length() != 0) {
    OpenProject(Configuration::Get(KEY_LAST_PROJECT).c_str());
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileBrowse()
{
  std::string filter = GenerateSupportedExtensionsFilter();

  char games_directory[MAX_PATH];
  GetGamesDirectory(games_directory);
  SetCurrentDirectory(games_directory);

  CFileDialog FileDialog(
    TRUE,
    "",
    NULL,
    OFN_FILEMUSTEXIST,
    filter.c_str());

  // set current directory on Win98/2000
  FileDialog.m_ofn.lpstrInitialDir = games_directory;

  if (FileDialog.DoModal() == IDOK) {
    char szDirectory[MAX_PATH];
    std::string t = FileDialog.GetPathName();
    strcpy(szDirectory, t.c_str());
    if (*strrchr(szDirectory, '\\'))
      *strrchr(szDirectory, '\\') = 0;

    CBrowseWindow* browse_window = new CBrowseWindow(szDirectory, "*");
    if (browse_window)
      m_DocumentWindows.push_back(browse_window);
  }
}

////////////////////////////////////////////////////////////////////////////////

#define FILE_NEW_HANDLER(name, construct)    \
  afx_msg void                               \
  CMainWindow::OnFileNew##name()             \
  {                                          \
    m_DocumentWindows.push_back(construct);  \
  }

FILE_NEW_HANDLER(Map,         new CMapWindow())
FILE_NEW_HANDLER(Script,      new CScriptWindow())
FILE_NEW_HANDLER(Spriteset,   new CSpritesetWindow())
FILE_NEW_HANDLER(Font,        new CFontWindow())
FILE_NEW_HANDLER(WindowStyle, new CWindowStyleWindow())
FILE_NEW_HANDLER(Image,       new CImageWindow())

#undef FILE_NEW_HANDLER

////////////////////////////////////////////////////////////////////////////////

#define FILE_OPEN_HANDLER(name, construct)                    \
  afx_msg void                                                \
  CMainWindow::OnFileOpen##name()                             \
  {                                                           \
    C##name##FileDialog Dialog(FDM_OPEN | FDM_MULTISELECT);   \
    if (Dialog.DoModal() == IDOK) {                           \
      POSITION pos = Dialog.GetStartPosition();               \
      while (pos != NULL) {                                   \
        CString path_ = Dialog.GetNextPathName(pos);          \
        const char* path = path_;                             \
        m_DocumentWindows.push_back(construct);               \
      }                                                       \
    }                                                         \
  }


FILE_OPEN_HANDLER(Map,         new CMapWindow(path))
FILE_OPEN_HANDLER(Script,      new CScriptWindow(path))
FILE_OPEN_HANDLER(Spriteset,   new CSpritesetWindow(path))
FILE_OPEN_HANDLER(Sound,       new CSoundWindow(path))
FILE_OPEN_HANDLER(Font,        new CFontWindow(path))
FILE_OPEN_HANDLER(WindowStyle, new CWindowStyleWindow(path))
FILE_OPEN_HANDLER(Image,       new CImageWindow(path))
FILE_OPEN_HANDLER(Animation,   new CAnimationWindow(path))

#undef FILE_OPEN_HANDLER

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileOpenTileset()
{
  CTilesetFileDialog Dialog(FDM_OPEN);

  if (Dialog.DoModal() == IDOK)
  {
    POSITION pos = Dialog.GetStartPosition();
    while (pos != NULL) {
      CString path_ = Dialog.GetNextPathName(pos);
      const char* path = path_;
      m_DocumentWindows.push_back(new CMapWindow(NULL, path));
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportImageToMap()
{
  // get name of image
  CImageFileDialog FileDialog(FDM_OPEN);
  if (FileDialog.DoModal() != IDOK)
    return;

  CString filename = FileDialog.GetPathName();

  CResizeDialog resize_dialog("Tile Dimensions", 16, 16);
  if (resize_dialog.DoModal() != IDOK)
    return;

  bool allow_duplicates = true;
  if (MessageBox("Do you want to remove duplicate tiles?", "Remove Duplicates", MB_ICONQUESTION | MB_YESNO) == IDYES) {
    allow_duplicates = false;
  }

  // load image
  CImage32 image;
  if (image.Load(filename) == false)
  {
    MessageBox("Error: Could not load image '" + FileDialog.GetFileName() + "'");
    return;
  }

  // build map from image
  sMap map;
  if (map.BuildFromImage(image, resize_dialog.GetWidth(), resize_dialog.GetHeight(), allow_duplicates) == false)
  {
    MessageBox("Error: Could not build map from image");
    return;
  }

  char* fn = new char[strlen(filename) + 10];
  if (fn == NULL)
    return;

  strcpy(fn, filename);

  strcpy(strrchr(fn, '.'), ".rmp");
  map.Save(fn);

  delete[] fn;

  MessageBox("Conversion successful");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportBitmapToRWS()
{
  CImageFileDialog InFileDialog(FDM_OPEN);
  if (InFileDialog.DoModal() == IDCANCEL)
    return;

  CWindowStyleFileDialog OutFileDialog(FDM_SAVE);
  if (OutFileDialog.DoModal() == IDCANCEL)
    return;

  sWindowStyle ws;
  if (ws.Import(InFileDialog.GetPathName(), CreateRGBA(255, 0, 255, 255)) == false)
  {
    MessageBox("Can't Import file, either file is invalid \nor not a 3x3 bitmap", "Error");
    return;
  }

  if (ws.Save(OutFileDialog.GetPathName()) == false)
  {
    MessageBox("Can't Save file!");
    return;
  }

  MessageBox("Import Successful!");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportBitmapToRSS()
{
  CImageFileDialog InFileDialog(FDM_OPEN);
  if (InFileDialog.DoModal() != IDOK)
    return;

  CSpritesetFileDialog OutFileDialog(FDM_SAVE);
  if (OutFileDialog.DoModal() != IDOK)
    return;

  CResizeDialog ResizeDialog("Frame Size", 16, 32);
  ResizeDialog.SetRange(1, 4096, 1, 4096);
  if (ResizeDialog.DoModal() != IDOK)
    return;

  sSpriteset sprite;
  if (sprite.Import_BMP(InFileDialog.GetPathName(), ResizeDialog.GetWidth(), ResizeDialog.GetHeight(), CreateRGBA(255, 0, 255, 255)) == false)
  {
    MessageBox("Can't Import file", "Error");
    return;
  }

  if (sprite.Save(OutFileDialog.GetPathName()) == false)
  {
    MessageBox("Can't Save file!");
    return;
  }

  MessageBox("Import Successful!");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportRM2KCharsetToRSS()
{
  CImageFileDialog InFileDialog(FDM_OPEN);
  if (InFileDialog.DoModal() != IDOK)
    return;

  CImage32 image;
  if (!image.Load(InFileDialog.GetPathName())) {
    MessageBox("Could not load image");
    return;
  }

  if (image.GetWidth() != 288 || image.GetHeight() != 256) {
    MessageBox("Invalid image size\nRM2K charsets are 288 by 256 images");
    return;
  }

  CStringDialog OutFilename("Save As", std::string(InFileDialog.GetFileTitle() + ".spriteset").c_str());
  if (OutFilename.DoModal() != IDOK) {
    return;
  } else {

    const int frame_width = 288/12;
    const int frame_height = 256/8;
    const int num_frames = 3;
    const int num_directions = 4;
    const int num_images = num_frames * num_directions;

    CFontGradientDialog transparent_color_dialog("Transparent color", "In", "Out", image.GetPixel(0, 0), CreateRGBA(image.GetPixel(0, 0).red, image.GetPixel(0, 0).green, image.GetPixel(0, 0).blue, 0));
    if (transparent_color_dialog.DoModal() != IDOK)
      return;

    const RGBA color1 = transparent_color_dialog.GetTopColor();
    const RGBA color2 = transparent_color_dialog.GetBottomColor();

    const char* base_filename = OutFilename.GetValue();
    char filename[MAX_PATH];

    sSpriteset sprite;

    // there are eight spritesets per chipset, 4 rows, 2 columns
    for (int sy = 0; sy < 2; sy++) {
      for (int sx = 0; sx < 4; sx++) { 

        // create the spriteset
        sprite.Create(frame_width, frame_height, num_images, num_directions, num_frames);
        sprite.SetDirectionName(0, "north");
        sprite.SetDirectionName(1, "east");
        sprite.SetDirectionName(2, "south");
        sprite.SetDirectionName(3, "west");
  
        for (int d = 0; d < num_directions; d++) {
          for (int f = 0; f < num_frames; f++) {
            CImage32& frame = sprite.GetImage(d * num_frames + f);
            int offset_x = (sx * frame_width * num_frames) + f * frame_width;
            int offset_y = (sy * frame_height * num_directions) + d * frame_height;
            for (int y = 0; y < frame_height; y++) {
              for (int x = 0; x < frame_height; x++) {
                frame.SetPixel(x, y, image.GetPixel(x + offset_x, y + offset_y));
              }
            }
            frame.ReplaceColor(color1, color2);
          }
        }

        for (int d = 0; d < num_directions; d++) {
          for (int f = 0; f < num_frames; f++) {
            sprite.SetFrameIndex(d, f, d * num_frames + f);
          }
        }

        // the following converts a four direction spriteset to an eight direction spriteset
        sprite.InsertDirection(1);
        sprite.InsertDirection(3);
        sprite.InsertDirection(5);
        sprite.InsertDirection(7);

        if (sprite.GetNumDirections() == 8) {
          sprite.SetDirectionName(1, "northeast");
          sprite.SetDirectionName(3, "southeast");
          sprite.SetDirectionName(5, "southwest");
          sprite.SetDirectionName(7, "northwest");

          for (int j = 0; j < num_directions; j++) {
            int d = (j * 2) + 1;
            for (int f = 0; f < num_frames; f++) {
              if (sprite.GetNumFrames(d) < num_frames)
                sprite.InsertFrame(d, f);
            }
          }

          for (int j = 0; j < num_directions; j++) {
            int base_index = j * num_frames;
            int d = (j * 2) + 1;

            for (int f = 0; f < num_frames; f++) {
              sprite.SetFrameIndex(d, f, base_index + f);
            }
          }

          // I'm assuming that the base is the bottom part of the frame
          sprite.SetBase(0, frame_height/2, frame_width, frame_height);
    
          sprintf(filename, "%s.%d.rss", base_filename, sy * 4 + sx);
          sprite.Save(filename);

        }

      }
    }
  }

  MessageBox("Charset Converted!");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportRM2KChipsetToRTS()
{
  CImageFileDialog InFileDialog(FDM_OPEN);
  if (InFileDialog.DoModal() != IDOK)
    return;

  CTilesetFileDialog OutFileDialog(FDM_SAVE);
  if (OutFileDialog.DoModal() != IDOK)
    return;

  bool allow_duplicates = true;

  CImage32 image;
  if (!image.Load(InFileDialog.GetPathName()) )
  {
    MessageBox("Can't Load image!");
    return;
  }

  if (image.GetWidth() != 480 || image.GetHeight() != 256) {
    MessageBox("Invalid image size\nRM2K chipsets are 480 by 256 images");
    return;
  }

  sTileset tileset;
  if (!tileset.BuildFromImage(image, 16, 16, allow_duplicates))
  {
    MessageBox("Can't convert image!");
    return;
  }

  CFontGradientDialog transparent_color_dialog("Transparent color", "In", "Out", image.GetPixel(image.GetWidth() - 1, 0), CreateRGBA(image.GetPixel(image.GetWidth() - 1, 0).red, image.GetPixel(image.GetWidth() - 1, 0).green, image.GetPixel(image.GetWidth() - 1, 0).blue, 0));
  if (transparent_color_dialog.DoModal() != IDOK)
    return;

  const RGBA color1 = transparent_color_dialog.GetTopColor();
  const RGBA color2 = transparent_color_dialog.GetBottomColor();

  for (int i = 0; i < tileset.GetNumTiles(); i++) {
    tileset.GetTile(i).ReplaceColor(color1, color2);
  }

  if (!tileset.Save(OutFileDialog.GetPathName()))
  {
    MessageBox("Can't save tileset!");
    return;
  }

  MessageBox("Image Converted!");
 
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportBitmapToRTS()
{
  CImageFileDialog InFileDialog(FDM_OPEN);
  if (InFileDialog.DoModal() != IDOK)
    return;

  CTilesetFileDialog OutFileDialog(FDM_SAVE);
  if (OutFileDialog.DoModal() != IDOK)
    return;

  CResizeDialog ResizeDialog("Tile size", 16, 16);
  if (ResizeDialog.DoModal() != IDOK)
    return;

  bool allow_duplicates = true;
  if (MessageBox("Do you want to remove duplicate tiles?", "Remove Duplicates", MB_ICONQUESTION | MB_YESNO) == IDYES) {
    allow_duplicates = false;
  }

  CImage32 image;
  if (!image.Load(InFileDialog.GetPathName()) )
  {
    MessageBox("Can't Load image!");
    return;
  }

  sTileset tileset;
  if (!tileset.BuildFromImage(image, ResizeDialog.GetWidth(), ResizeDialog.GetHeight(), allow_duplicates))
  {
    MessageBox("Can't convert image!");
    return;
  }

  if (!tileset.Save(OutFileDialog.GetPathName()))
  {
    MessageBox("Can't save tileset!");
    return;
  }

  MessageBox("Image Converted!");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportVergeFontTemplate()
{
  CImageFileDialog image_dialog(FDM_OPEN);
  if (image_dialog.DoModal() != IDOK) {
    return;
  }

  CFontFileDialog font_dialog(FDM_SAVE);
  if (font_dialog.DoModal() != IDOK) {
    return;
  }

  CResizeDialog resize_dialog("Font character size", 8, 12);
  if (resize_dialog.DoModal() != IDOK) {
    return;
  }

  sFont font;
  if (!font.ImportVergeTemplate(
        image_dialog.GetPathName(),
        resize_dialog.GetWidth(),
        resize_dialog.GetHeight())) {
    MessageBox("Could not import VERGE font template", "Import VERGE font template");
    return;
  }

  if (!font.Save(font_dialog.GetPathName())) {
    MessageBox("Could not save font", "Import VERGE font template");
    return;
  }

  MessageBox("Font imported successfully!", "Import VERGE font template");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportVergeMap()
{
  #define all_files "All Files (*.*)|*.*||"

  int flags = OFN_FILEMUSTEXIST;
  CFileDialog verge_map_dialog    (TRUE, "map", NULL, flags, "VERGE Map Files (*.map)|*.map|"     all_files, this);
  CFileDialog verge_tileset_dialog(TRUE, "vsp", NULL, flags, "VERGE Tileset Files (*.vsp)|*.vsp|" all_files, this);
  CMapFileDialog map_dialog(FDM_SAVE);

  #undef all_files

  // load VERGE map
  if (verge_map_dialog.DoModal() != IDOK) {
    return;
  }

  // load VERGE tileset
  if (verge_tileset_dialog.DoModal() != IDOK) {
    return;
  }

  // import
  sMap map;
  if (!map.Import_VergeMAP(
        verge_map_dialog.GetPathName(),
        verge_tileset_dialog.GetPathName()
      )
  ) {
    MessageBox("Could not import Verge map");
    return;
  }

  // save
  if (map_dialog.DoModal() == IDOK) {
    if (map.Save(map_dialog.GetPathName())) {
      MessageBox("Import successful!");
    } else {
      MessageBox("Could not save Sphere map");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportVergeSpriteset()
{
  CFileDialog verge_spriteset_dialog(TRUE, "chr", NULL, OFN_FILEMUSTEXIST,
    "VERGE Spriteset Files (*.chr)|*.chr|All Files (*.*)|*.*||", this);
  CFileDialog verge_palette_dialog(TRUE, "pal", NULL, OFN_FILEMUSTEXIST,
    "VERGE Palette Files (*.pal)|*.pal|All Files (*.*)|*.*||", this);
  CSpritesetFileDialog spriteset_dialog(FDM_SAVE);

  // load VERGE spriteset
  if (verge_spriteset_dialog.DoModal() != IDOK) {
    return;
  }

  // load VERGE palette
  if (verge_palette_dialog.DoModal() != IDOK) {
    return;
  }

  // import spriteset
  sSpriteset spriteset;
  if (!spriteset.Import_CHR(
    verge_spriteset_dialog.GetPathName(),
    verge_palette_dialog.GetPathName()
  )) {
    MessageBox("Could not import VERGE spriteset");
    return;
  }

  // save
  if (spriteset_dialog.DoModal() == IDOK) {
    if (spriteset.Save(spriteset_dialog.GetPathName())) {
      MessageBox("Import successful!");
    } else {
      MessageBox("Could not save Sphere spriteset");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportMergeRGBA()
{
  CImageFileDialog rgb_dialog  (FDM_OPEN, "Select RGB Image");
  CImageFileDialog alpha_dialog(FDM_OPEN, "Select Alpha Image");
  CImageFileDialog out_dialog  (FDM_SAVE, "Select Output Image");

  if (rgb_dialog.DoModal()   != IDOK ||
      alpha_dialog.DoModal() != IDOK ||
      out_dialog.DoModal()   != IDOK) {
    return;
  }

  CImage32 rgb;
  CImage32 alpha;
  
  if (!rgb.Load(rgb_dialog.GetPathName())) {
    MessageBox("Can't load image '" + rgb_dialog.GetFileName() + "'");
    return;
  }

  if (!alpha.Load(alpha_dialog.GetPathName())) {
    MessageBox("Can't load image '" + alpha_dialog.GetFileName() + "'");
    return;
  }

  if (rgb.GetWidth() != alpha.GetWidth() ||
      rgb.GetHeight() != alpha.GetHeight()) {
    MessageBox("RGB and alpha images aren't same size");
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

  if (!out.Save(out_dialog.GetPathName())) {
    MessageBox("Can't save image '" + out_dialog.GetFileName() + "'");
    return;
  }

  MessageBox("Image merged successfully");
}

////////////////////////////////////////////////////////////////////////////////

static bool WindowsFontToSphereFont(LOGFONT lf, COLORREF color, sFont* sphere_font)
{
  // grab the font
  HDC dc = CreateCompatibleDC(NULL);
  HFONT font = CreateFontIndirect(&lf);
  HFONT old_font = (HFONT)SelectObject(dc, font);

  if (!sphere_font->Create(256))
    return false;
  
  for (int i = 0; i < 256; i++) {
    char c = (char)i;
    SIZE size;
    if (GetTextExtentPoint32(dc, &c, 1, &size) == FALSE)
      return false;

    // grab the character
    CDIBSection* dib = new CDIBSection(size.cx, size.cy, 32);
    if (dib == NULL)
      return false;

    memset(dib->GetPixels(), 255, size.cx * size.cy * 4); // opaque white

    RECT rect = { 0, 0, size.cx, size.cy };

    SetTextColor(dib->GetDC(), 0);  // black
    SetBkMode(dib->GetDC(), TRANSPARENT);
    HFONT old_font = (HFONT)SelectObject(dib->GetDC(), font);
    DrawText(dib->GetDC(), &c, 1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
    SelectObject(dib->GetDC(), old_font);

    // put it into the font
    int width = size.cx + 1;
    sphere_font->GetCharacter(i).Resize(width, size.cy);
    
    if (sphere_font->GetCharacter(i).GetWidth() != width
     || sphere_font->GetCharacter(i).GetHeight() != size.cy) {
      delete dib;
      return false;
    }

    memset(sphere_font->GetCharacter(i).GetPixels(), 0, width * size.cy * 4);

    BGRA* src = (BGRA*)dib->GetPixels();
    RGBA* dst = sphere_font->GetCharacter(i).GetPixels();
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

  return true;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileImportWindowsFont()
{
  // windows font dialog
  CFontDialog font_dialog(NULL, CF_FORCEFONTEXIST | CF_EFFECTS | CF_SCREENFONTS);
  if (font_dialog.DoModal() != IDOK) {
    return;
  }

  COLORREF color = font_dialog.GetColor();

  // get the font from the dialog
  LOGFONT lf;
  font_dialog.GetCurrentFont(&lf);

  sFont sphere_font;
  if (!WindowsFontToSphereFont(lf, color, &sphere_font))
    return;

  // save the Sphere font
  CFontFileDialog file_dialog(FDM_SAVE, "Select Output Font");
  if (file_dialog.DoModal() != IDOK) {
    return;
  }

  if (!sphere_font.Save(file_dialog.GetPathName())) {
    MessageBox("Could not save font");
  }

}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnFileSaveAll()
{
  for (int i = 0; i < m_DocumentWindows.size(); i++) {
    CDocumentWindow* dw = m_DocumentWindows[i];
    if (dw->IsSaveable()) {
      CSaveableDocumentWindow* sdw = (CSaveableDocumentWindow*)dw;
      sdw->Save();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

#define PROJECT_INSERT_HANDLER(type, group_type)                        \
afx_msg void                                                            \
CMainWindow::OnProjectInsert##type()                                    \
{                                                                       \
  InsertProjectFile(                                                    \
    &C##type##FileDialog(FDM_OPEN | FDM_MAYNOTEXIST), group_type);      \
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

afx_msg void
CMainWindow::OnProjectRefresh()
{
  if (m_ProjectWindow) {
    m_ProjectWindow->Update();
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnProjectRunSphere()
{
  char szCommandLine[MAX_PATH + 80];
  strcpy(szCommandLine, g_SphereDirectory.c_str());
  strcat(szCommandLine, "\\engine.exe -game ");
  strcat(szCommandLine, "\"");
  strcat(szCommandLine, m_Project.GetDirectory());
  strcat(szCommandLine, "\"");

  if (true) {
    char filename[MAX_PATH];
    memset(filename, 0, MAX_PATH);

    // find the currently open map
    // todo: fix this, we shouldn't just be checking if the window is visible
    for (int i = 0; i < m_DocumentWindows.size(); i++) {
      const CDocumentWindow* dw = m_DocumentWindows[i];
      if (dw->IsWindowVisible()) {
        strcpy(filename, dw->GetFilename());
        strcpy(filename, strrchr(filename, '\\') + 1);
      }
    }

    std::string __filename__(strlwr(filename)); // make the extension lowercase

    if (__filename__.rfind(".rmp") == __filename__.size() - 4) {
      strcat(szCommandLine, " -parameters ");
      strcat(szCommandLine, "\"'");
      strcat(szCommandLine, filename);   
      strcat(szCommandLine, "'\"");
    }
  }

  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  
  char sphere_directory[MAX_PATH];
  strcpy(sphere_directory, g_SphereDirectory.c_str());

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
    MessageBox("Error: Could not execute Sphere engine");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnProjectConfigureSphere()
{
  char sphere_directory[MAX_PATH];
  strcpy(sphere_directory, g_SphereDirectory.c_str());

  char szCommandLine[MAX_PATH + 80];
  strcpy(szCommandLine, sphere_directory);
  strcat(szCommandLine, "\\config.exe");

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
    MessageBox("Error: Could not configure Sphere");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnProjectPackageGame()
{
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);
  SetCurrentDirectory(m_Project.GetDirectory());

  // go into game directory and add all files
  std::list<std::string> files;
  EnumerateFiles("*", "", files);

  CPackage package;
  std::list<std::string>::iterator i;
  for (i = files.begin(); i != files.end(); i++) {
    package.AddFile(i->c_str());
  }

  // TODO:  show a wait dialog (no cancel)

  CPackageFileDialog dialog(FDM_SAVE);
  dialog.m_ofn.Flags |= OFN_NOCHANGEDIR;

  if (dialog.DoModal() == IDOK) {
    // write the package!
    if (!package.Write(dialog.GetPathName())) {
      MessageBox("Package creation failed", "Package Game");
    } else {
      MessageBox("Package creation succeeded!", "Package Game");
    }
  }

  SetCurrentDirectory(old_directory);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnProjectMakeInstaller()
{
  char old_directory[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, old_directory);
  SetCurrentDirectory(m_Project.GetDirectory());

  bool stand_alone = false;

  FILE* file = fopen("installer.nsi", "wb+");
  if (file) {

    fprintf (file, "; This is a NSIS (nullsoft install script) for %s by %s\n\n", m_Project.GetGameTitle(), m_Project.GetAuthor());

    fprintf (file, "Name \"%s\"\n", m_Project.GetGameTitle());
    fprintf (file, "OutFile \"%s.exe\"\n", m_Project.GetDirectory());

    if (stand_alone) {
      fprintf (file, "InstallDir \"$PROGRAMFILES\\%s\"\n", m_Project.GetGameTitle());
    }
    else {
      fprintf (file, "InstallDir \"$PROGRAMFILES\\sphere\"\n");
    }

    if (stand_alone) {
      fprintf (file, "DirText \"Please select the path where you want to install$\\r%s\"\n", m_Project.GetGameTitle());
    }
    else {
      fprintf (file, "DirText \"Please select your Sphere path below$\\r(You will be able to proceed when Sphere is detected):\"\n");
    }

    fprintf (file, "DirShow show\n");

    fprintf (file, "\n;-----------------------------\n\n");

    fprintf (file, "Section \"\"\n\n");

    if (stand_alone) {
      fprintf (file, "  !define InstallDirectory \"$INSTDIR\"\n");
    }
    else {

      fprintf (file, "  !define SphereDirectory \"$INSTDIR\"\n");
      fprintf (file, "  !define InstallDirectory \"$INSTDIR\\games\\%s\"\n", m_Project.GetGameTitle());

      fprintf (file, "\n  IfFileExists \"${SphereDirectory}\\engine.exe\" okay\n");
      fprintf (file, "  MessageBox MB_OK \"Can not find sphere $\\r$\\\"${SphereDirectory}\\engine.exe$\\\"\"\n");
      fprintf (file, "    Quit\n");
      fprintf (file, "  okay:\n\n");
    }

    if (stand_alone) {
      const char* engine_file_list[] = {"audiere.dll", "corona.dll", "engine.exe", "js32.dll", "libmng.dll", "zlib.dll"};
      const char* editor_file_list[] = {"editor.exe", "SciLexer.dll", "Scintilla.dll"};
      const int engine_file_list_size = sizeof(engine_file_list) / sizeof(*engine_file_list);
      const int editor_file_list_size = sizeof(editor_file_list) / sizeof(*editor_file_list);

      fprintf (file, "  SetOutPath \"${InstallDirectory}\"\n");

      if (true) {
        fprintf (file, "  File ");
        for (int i = 0; i < engine_file_list_size; i++)
          fprintf (file, "\"..\\..\\%s\"", engine_file_list[i]);
        fprintf (file, "\n");
      }

      if (false) {
        fprintf (file, "  File ");
        for (int i = 0; i < editor_file_list_size; i++)
          fprintf (file, "\"..\\..\\%s\"", editor_file_list[i]);
        fprintf (file, "\n");
      }

      fprintf (file, "  File /r ..\\..\\system\n\n");
      fprintf (file, "  SetOutPath \"${InstallDirectory}\\startup\"\n");
    }
    else {
      fprintf (file, "  SetOutPath \"${InstallDirectory}\"\n");
    }

    fprintf (file, "  File /r * ");
    for (int i = 0; i < NUM_GROUP_TYPES; i++) {
      if (m_Project.GetItemCount(i)) {
        fprintf (file, "%s ", m_Project.GetGroupDirectory(i));
      }
    }

    fprintf (file, "\n");

    if (stand_alone) {
      fprintf (file, "\n");
      fprintf (file, "  CreateDirectory \"$SMPROGRAMS\\%s\"\n", m_Project.GetGameTitle());
      fprintf (file, "  CreateShortCut \"$SMPROGRAMS\\%s\\%s.lnk\" \"${InstallDirectory}\\engine.exe\" \"%s\"\n", m_Project.GetGameTitle(), m_Project.GetGameTitle(), "icon.ico");
      fprintf (file, "  CreateShortCut \"$SMPROGRAMS\\%s\\config.lnk\" \"${InstallDirectory}\\config.exe\"\n");
      fprintf (file, "\n");
    }
    else {
      fprintf (file, "\n");
      fprintf (file, "  !define SphereStartMenuDirectory \"Sphere\"\n");
      fprintf (file, "\n");
      fprintf (file, "  SetOutPath \"${SphereDirectory}\"\n");
      fprintf (file, "  CreateDirectory \"$SMPROGRAMS\\${SphereStartMenuDirectory}\\games\\%s\"\n", m_Project.GetGameTitle());
      fprintf (file, "  CreateShortCut \"$SMPROGRAMS\\${SphereStartMenuDirectory}\\games\\%s\\%s.lnk\" \"${SphereDirectory}\\engine.exe\" `-game \"%s\"` \"%s\"\n", m_Project.GetGameTitle(), m_Project.GetGameSubDirectory(), m_Project.GetGameSubDirectory(), "icon.ico");
      fprintf (file, "\n");
    }

    fprintf (file, "SectionEnd\n");
    fclose(file);

  }

  SetCurrentDirectory(old_directory);
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnWindowCloseAll()
{
  for (int i = 0; i < m_DocumentWindows.size(); i++) {
    CDocumentWindow* dw = m_DocumentWindows[i];
    if (dw->Close() == false) {
      break;
    }

    dw->DestroyWindow();
  }
}

////////////////////////////////////////////////////////////////////////////////

void OpenURL(const std::string& url, const std::string& label)
{
  if ((int)ShellExecute(NULL, "open", url.c_str(), 0, 0, SW_SHOW) <= 32) {
    MessageBox(
      NULL,
      ("Could not open web site.  Visit " + url).c_str(),
      label.c_str(),
      MB_OK);
  }
}

afx_msg void
CMainWindow::OnHelpSphereSite()
{
  OpenURL("http://sphere.sourceforge.net/", "Open Sphere Site");
}

afx_msg void
CMainWindow::OnHelpSphereFAQ()
{
  OpenURL("http://sphere.sourceforge.net/modules.php?op=modload&name=FAQ&file=index",
          "Open Frequently Asked Questions");
}

afx_msg void
CMainWindow::OnHelpAegisKnightsSite()
{
  OpenURL("http://aegisknight.org/sphere", "Open AegisKnight's Site");
}

afx_msg void
CMainWindow::OnHelpFliksSite()
{
  OpenURL("http://sphere.sourceforge.net/flik/docs", "Open Flik's Tutorial Site");
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnHelpLocalDocumentation()
{
  std::string docdir = g_SphereDirectory + "\\docs";
  if ((int)ShellExecute(m_hWnd, "open", docdir.c_str(), 0, 0, SW_SHOW) <= 32) {
    MessageBox("Could not open documentation directory.", "Local Documentation");
  }
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnHelpAbout()
{
  char message[1024];
  sprintf(message,
    "Sphere Development Environment\n"
    SPHERE_VERSION "\n"
    "\n"
    "Chad Austin (c) 1999-2003\n"
    "Additional code by Jacky Chong (Darklich) and Brian Robb (Flik)\n"
    "Icons by DRosen and Khadgar\n"
    "\n"
    "\n"
    __DATE__ "\n"
    __TIME__ "\n"
    "\n"
    "JavaScript: %s\n"
    "zlib: %s\n"
    "libmng: DLL %s - header %s\n"
    "Corona: %s\n"
    "Audiere: %s\n"
#ifdef USE_SIZECBAR
		"CSizingControlBar: %s\n"
#endif
		,
    JS_GetImplementationVersion(),
    zlibVersion(),
    mng_version_text(), MNG_VERSION_TEXT,
    corona::GetVersion(),
    audiere::GetVersion()
#ifdef USE_SIZECBAR
		, SIZECBAR_VERSION
#endif
		);

  MessageBox(message, "About");
}

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

afx_msg LRESULT
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
        return 0;
      }
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg LRESULT
CMainWindow::OnRefreshProject(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
  m_Project.RefreshItems();
  if (m_ProjectWindow) {
    m_ProjectWindow->Update();
  }

  return 0;
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

void
CMainWindow::OnUpdatePaletteMenu(CCmdUI * ui)
{
	if (ui->m_pSubMenu == NULL) return;

	// remove the old palette menu	
	while(ui->m_pSubMenu->GetMenuItemCount() > 0)
	{
		ui->m_pSubMenu->RemoveMenu(0, MF_BYPOSITION);
	}

  // add a new one
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
#ifdef USE_SIZECBAR
				if (palette->IsVisible()) {
#else
        if (palette->IsWindowVisible()) {
#endif
          visible = MF_CHECKED;
        }

        ui->m_pSubMenu->AppendMenu(visible, PALETTE_COMMAND + i, title);
      }

      empty = (window->GetNumPalettes() == 0);
    }

  }

  // if nothing was added, make an empty palette menu
  if (empty) {
    ui->m_pSubMenu->AppendMenu(0, ID_VIEW_PALETTES, "...");
    ui->m_pSubMenu->EnableMenuItem(0, MF_BYPOSITION | MF_GRAYED);
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnViewPalette(UINT id)
{
  ViewPalette(id - PALETTE_COMMAND);
}

////////////////////////////////////////////////////////////////////////////////

void 
CMainWindow::ViewPalette(int paletteNum)
{
  // toggle the visibility of the palette

  CWnd* child = MDIGetActive();
  if (child) {
    if (GetWindowLong(child->m_hWnd, GWL_USERDATA) & WA_DOCUMENT_WINDOW) {
      CDocumentWindow* window = (CDocumentWindow*)child;

      // find the palette to toggle
      CPaletteWindow* palette = window->GetPalette(paletteNum);
#ifdef USE_SIZECBAR
			palette->ShowPalette(!palette->IsVisible());
#else
      palette->ShowPalette(!palette->IsWindowVisible());
#endif
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void
CMainWindow::OnUpdateViewProject(CCmdUI * ui)
{
	ui->Enable(m_ProjectOpen);
	if (m_ProjectOpen)
	{
#ifdef USE_SIZECBAR
	ui->SetCheck(m_ProjectWindow->IsVisible() ? 1 : 0);
#else
	ui->SetCheck(m_ProjectWindow->IsWindowVisible() ? 1 : 0);
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////

afx_msg void
CMainWindow::OnViewProject()
{
#ifdef USE_SIZECBAR
	m_ProjectWindow->ShowPalette(!m_ProjectWindow->IsVisible());
#else
	m_ProjectWindow->ShowPalette(!m_ProjectWindow->IsWindowVisible());
#endif
}

////////////////////////////////////////////////////////////////////////////////

