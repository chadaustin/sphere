#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP


#pragma warning(disable : 4786)  // identifier too long


#include <vector>
#include <afxwin.h>
#include <afxext.h>
#include "Project.hpp"
#include "SwatchServer.hpp"


class CDocumentWindow;  // DocumentWindow.hpp
class CProjectWindow;   // ProjectWindow.hpp


// send this to the main window to insert a file
#define WM_INSERT_PROJECT_FILE (WM_APP + 923) // wparam = 0, path = (char*)lparam
#define WM_REFRESH_PROJECT     (WM_APP + 924) // wparam = 0, lparam = 0


class CMainWindow : public CMDIFrameWnd
{
public:
  CMainWindow();
  
  BOOL Create();

  void CreateProject(const char* projectname, const char* gametitle);
  void OpenProject(const char* filename);
  void CloseProject();

  void OpenGameSettings();
  void OpenGameFile(const char* filename);
  void OpenDocumentWindow(int grouptype, const char* filename);

private:
  bool AddToDirectory(const char* pathname, const char* sub_directory);
  bool CheckDirectory(const char* filename, const char* sub_directory);
  void InsertProjectFile(CFileDialog* file_dialog, int grouptype, const char* predefined_path = NULL);
  void GetGamesDirectory(char games_directory[MAX_PATH]);
  void UpdateMenu();
  void UpdatePaletteMenu();

  afx_msg void OnDropFiles(HDROP hDropInfo);
  afx_msg void OnClose();

  afx_msg void OnFileOpen();

  afx_msg void OnFileNewProject();
  afx_msg void OnFileOpenProject();
  afx_msg void OnFileCloseProject();
  afx_msg void OnFileOpenLastProject();

  afx_msg void OnFileNewMap();
  afx_msg void OnFileNewSpriteset();
  afx_msg void OnFileNewScript();
  afx_msg void OnFileNewFont();
  afx_msg void OnFileNewWindowStyle();
  afx_msg void OnFileNewImage();
  afx_msg void OnFileNewDoodad();

  afx_msg void OnFileOpenMap();
  afx_msg void OnFileOpenSpriteset();
  afx_msg void OnFileOpenScript();
  afx_msg void OnFileOpenSound();
  afx_msg void OnFileOpenFont();
  afx_msg void OnFileOpenWindowStyle();
  afx_msg void OnFileOpenImage();
  afx_msg void OnFileOpenAnimation();
  afx_msg void OnFileOpenDoodad();

  afx_msg void OnFileImportImageToMap();
  afx_msg void OnFileImportBitmapToRWS();
  afx_msg void OnFileImportBitmapToRSS();
  afx_msg void OnFileImportBitmapToRTS();
  afx_msg void OnFileImportVergeFontTemplate();
  afx_msg void OnFileImportVergeMap();
  afx_msg void OnFileImportVergeSpriteset();
  afx_msg void OnFileImportMergeRGBA();
  afx_msg void OnFileImportWindowsFont();

  afx_msg void OnFileSaveAll();

  afx_msg void OnFileOptions();

  afx_msg void OnProjectInsertMap();
  afx_msg void OnProjectInsertSpriteset();
  afx_msg void OnProjectInsertScript();
  afx_msg void OnProjectInsertSound();
  afx_msg void OnProjectInsertFont();
  afx_msg void OnProjectInsertWindowStyle();
  afx_msg void OnProjectInsertImage();
  afx_msg void OnProjectInsertAnimation();
  afx_msg void OnProjectInsertDoodad();

  afx_msg void OnProjectRefresh();
  afx_msg void OnProjectRunSphere();
  afx_msg void OnProjectConfigureSphere();
  afx_msg void OnProjectPackageGame();

  afx_msg void OnWindowCloseAll();

  afx_msg void OnHelpSphereSite();
  afx_msg void OnHelpAegisKnightsSite();
  afx_msg void OnHelpFliksSite();
  afx_msg void OnHelpLocalDocumentation();
  afx_msg void OnHelpAbout();

  afx_msg BOOL OnNeedText(UINT id, NMHDR* nmhdr, LRESULT* result);

  afx_msg void OnUpdateOpenLastProject(CCmdUI* cmdui);
  afx_msg void OnUpdateProjectCommand(CCmdUI* cmdui);
  afx_msg void OnUpdateSaveCommand(CCmdUI* cmdui);
  afx_msg void OnUpdateSaveAllCommand(CCmdUI* cmdui);
  afx_msg void OnUpdateWindowCloseAll(CCmdUI* cmdui);

  afx_msg void OnInsertProjectFile(WPARAM wparam, LPARAM lparam);
  afx_msg void OnRefreshProject(WPARAM wparam, LPARAM lparam);

  afx_msg LRESULT OnDocumentWindowClosing(WPARAM wparam, LPARAM lparam);
  afx_msg LRESULT OnSetChildMenu(WPARAM wparam, LPARAM lparam);
  afx_msg LRESULT OnClearChildMenu(WPARAM wparam, LPARAM lparam);
  afx_msg LRESULT OnUpdatePaletteMenu(WPARAM wparam, LPARAM lparam);
  afx_msg void OnViewPalette(UINT id);

private:
  CToolBar   m_ToolBar;
  CStatusBar m_StatusBar;

  bool     m_ProjectOpen;
  CProject m_Project;

  CProjectWindow* m_ProjectWindow;

  // manage pointers to document windows so we can enumerate and send messages to them
  std::vector<CDocumentWindow*> m_DocumentWindows;

  int m_ChildMenuResource;

  DECLARE_MESSAGE_MAP()
};


#endif
